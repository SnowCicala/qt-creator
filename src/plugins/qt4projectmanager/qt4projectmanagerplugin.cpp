/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** No Commercial Usage
**
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**************************************************************************/

#include "qt4projectmanagerplugin.h"

#include "qt4projectmanager.h"
#include "qmakestep.h"
#include "makestep.h"
#include "wizards/consoleappwizard.h"
#include "wizards/guiappwizard.h"
#include "wizards/mobileappwizard.h"
#include "wizards/librarywizard.h"
#include "wizards/testwizard.h"
#include "wizards/emptyprojectwizard.h"
#include "wizards/qmlstandaloneappwizard.h"
#include "customwidgetwizard/customwidgetwizard.h"
#include "profileeditorfactory.h"
#include "qt4projectmanagerconstants.h"
#include "qt4project.h"
#include "qt4runconfiguration.h"
#include "profileeditor.h"
#include "profilereader.h"
#include "qtversionmanager.h"
#include "qtoptionspage.h"
#include "externaleditors.h"
#include "gettingstartedwelcomepage.h"

#include "qt-maemo/maemomanager.h"
#include "qt-s60/s60manager.h"

#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/icore.h>
#include <extensionsystem/pluginmanager.h>
#include <projectexplorer/buildmanager.h>
#include <projectexplorer/project.h>
#include <projectexplorer/session.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectnodes.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/editormanager/editormanager.h>
#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorconstants.h>

#ifdef WITH_TESTS
#    include <QTest>
#endif

#include <QtCore/QtPlugin>

using namespace Qt4ProjectManager::Internal;
using namespace Qt4ProjectManager;
using ProjectExplorer::Project;

Qt4ProjectManagerPlugin::~Qt4ProjectManagerPlugin()
{
    //removeObject(m_embeddedPropertiesPage);
    //delete m_embeddedPropertiesPage;

    removeObject(m_proFileEditorFactory);
    delete m_proFileEditorFactory;
    removeObject(m_qt4ProjectManager);
    delete m_qt4ProjectManager;
    removeObject(m_welcomePage);
    delete m_welcomePage;
}

bool Qt4ProjectManagerPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)
    m_projectContext = Core::Context(Qt4ProjectManager::Constants::PROJECT_ID);

    ProFileParser::initialize();
    ProFileEvaluator::initialize();

    Core::ICore *core = Core::ICore::instance();
    if (!core->mimeDatabase()->addMimeTypes(QLatin1String(":qt4projectmanager/Qt4ProjectManager.mimetypes.xml"), errorMessage))
        return false;

    m_projectExplorer = ProjectExplorer::ProjectExplorerPlugin::instance();
    Core::ActionManager *am = core->actionManager();

    QtVersionManager *mgr = new QtVersionManager;
    addAutoReleasedObject(mgr);
    addAutoReleasedObject(new QtOptionsPage);

    m_welcomePage = new GettingStartedWelcomePage;
    addObject(m_welcomePage);
    connect(mgr, SIGNAL(updateExamples(QString,QString,QString)),
            m_welcomePage, SLOT(updateExamples(QString,QString,QString)));

    //create and register objects
    m_qt4ProjectManager = new Qt4Manager(this);
    addObject(m_qt4ProjectManager);

    TextEditor::TextEditorActionHandler *editorHandler
            = new TextEditor::TextEditorActionHandler(Constants::C_PROFILEEDITOR,
                  TextEditor::TextEditorActionHandler::UnCommentSelection);

    m_proFileEditorFactory = new ProFileEditorFactory(m_qt4ProjectManager, editorHandler);
    addObject(m_proFileEditorFactory);

    addAutoReleasedObject(new EmptyProjectWizard);

    GuiAppWizard *guiWizard = new GuiAppWizard;
    addAutoReleasedObject(guiWizard);

    ConsoleAppWizard *consoleWizard = new ConsoleAppWizard;
    addAutoReleasedObject(consoleWizard);

    MobileAppWizard *mobileWizard = new MobileAppWizard;
    addAutoReleasedObject(mobileWizard);

    addAutoReleasedObject(new QmlStandaloneAppWizard());

    LibraryWizard *libWizard = new LibraryWizard;
    addAutoReleasedObject(libWizard);
    addAutoReleasedObject(new TestWizard);
    addAutoReleasedObject(new CustomWidgetWizard);

    CustomQt4ProjectWizard::registerSelf();

    addAutoReleasedObject(new QMakeStepFactory);
    addAutoReleasedObject(new MakeStepFactory);

    addAutoReleasedObject(new Qt4RunConfigurationFactory);

#ifdef Q_OS_MAC
    addAutoReleasedObject(new MacDesignerExternalEditor);
#else
    addAutoReleasedObject(new DesignerExternalEditor);
#endif
    addAutoReleasedObject(new LinguistExternalEditor);

    addAutoReleasedObject(new S60Manager);
    addAutoReleasedObject(new MaemoManager);

    new ProFileCacheManager(this);

    // TODO reenable
    //m_embeddedPropertiesPage = new EmbeddedPropertiesPage;
    //addObject(m_embeddedPropertiesPage);

    //menus
    Core::ActionContainer *mbuild =
            am->actionContainer(ProjectExplorer::Constants::M_BUILDPROJECT);
    Core::ActionContainer *mproject =
            am->actionContainer(ProjectExplorer::Constants::M_PROJECTCONTEXT);
    Core::ActionContainer *msubproject =
            am->actionContainer(ProjectExplorer::Constants::M_SUBPROJECTCONTEXT);

    //register actions
    Core::Command *command;

    QIcon qmakeIcon(QLatin1String(":/qt4projectmanager/images/run_qmake.png"));
    qmakeIcon.addFile(QLatin1String(":/qt4projectmanager/images/run_qmake_small.png"));
    m_runQMakeAction = new QAction(qmakeIcon, tr("Run qmake"), this);
    command = am->registerAction(m_runQMakeAction, Constants::RUNQMAKE, m_projectContext);
    mbuild->addAction(command, ProjectExplorer::Constants::G_BUILD_PROJECT);
    connect(m_runQMakeAction, SIGNAL(triggered()), m_qt4ProjectManager, SLOT(runQMake()));

    m_runQMakeActionContextMenu = new QAction(qmakeIcon, tr("Run qmake"), this);
    command = am->registerAction(m_runQMakeActionContextMenu, Constants::RUNQMAKECONTEXTMENU, m_projectContext);
    command->setAttribute(Core::Command::CA_Hide);
    mproject->addAction(command, ProjectExplorer::Constants::G_PROJECT_BUILD);
    msubproject->addAction(command, ProjectExplorer::Constants::G_PROJECT_BUILD);
    connect(m_runQMakeActionContextMenu, SIGNAL(triggered()), m_qt4ProjectManager, SLOT(runQMakeContextMenu()));

    QIcon buildIcon(ProjectExplorer::Constants::ICON_BUILD);
    buildIcon.addFile(ProjectExplorer::Constants::ICON_BUILD_SMALL);
    m_buildSubProjectContextMenu = new QAction(buildIcon, tr("Build"), this);
    command = am->registerAction(m_buildSubProjectContextMenu, Constants::BUILDSUBDIR, m_projectContext);
    command->setAttribute(Core::Command::CA_Hide);
    msubproject->addAction(command, ProjectExplorer::Constants::G_PROJECT_BUILD);
    connect(m_buildSubProjectContextMenu, SIGNAL(triggered()), m_qt4ProjectManager, SLOT(buildSubDirContextMenu()));

    QIcon rebuildIcon(ProjectExplorer::Constants::ICON_REBUILD);
    rebuildIcon.addFile(ProjectExplorer::Constants::ICON_REBUILD_SMALL);
    m_rebuildSubProjectContextMenu = new QAction(rebuildIcon, tr("Rebuild"), this);
    command = am->registerAction(m_rebuildSubProjectContextMenu, Constants::REBUILDSUBDIR, m_projectContext);
    command->setAttribute(Core::Command::CA_Hide);
    msubproject->addAction(command, ProjectExplorer::Constants::G_PROJECT_BUILD);
    connect(m_rebuildSubProjectContextMenu, SIGNAL(triggered()), m_qt4ProjectManager, SLOT(rebuildSubDirContextMenu()));

    QIcon cleanIcon(ProjectExplorer::Constants::ICON_CLEAN);
    cleanIcon.addFile(ProjectExplorer::Constants::ICON_CLEAN_SMALL);
    m_cleanSubProjectContextMenu = new QAction(cleanIcon, tr("Clean"), this);
    command = am->registerAction(m_cleanSubProjectContextMenu, Constants::CLEANSUBDIR, m_projectContext);
    command->setAttribute(Core::Command::CA_Hide);
    msubproject->addAction(command, ProjectExplorer::Constants::G_PROJECT_BUILD);
    connect(m_cleanSubProjectContextMenu, SIGNAL(triggered()), m_qt4ProjectManager, SLOT(cleanSubDirContextMenu()));

    connect(m_projectExplorer,
            SIGNAL(aboutToShowContextMenu(ProjectExplorer::Project*, ProjectExplorer::Node*)),
            this, SLOT(updateContextMenu(ProjectExplorer::Project*, ProjectExplorer::Node*)));

    connect(m_projectExplorer->buildManager(), SIGNAL(buildStateChanged(ProjectExplorer::Project *)),
            this, SLOT(buildStateChanged(ProjectExplorer::Project *)));
    connect(m_projectExplorer, SIGNAL(currentProjectChanged(ProjectExplorer::Project *)),
            this, SLOT(currentProjectChanged()));

    Core::ActionContainer *contextMenu = am->createMenu(Qt4ProjectManager::Constants::M_CONTEXT);

    Core::Command *cmd;

    cmd = am->command(TextEditor::Constants::UN_COMMENT_SELECTION);
    contextMenu->addAction(cmd);

    Core::Context proFileEditorContext = Core::Context(Qt4ProjectManager::Constants::PROJECT_ID);

    QAction *addLibrary = new QAction(tr("Add Library..."), this);
    cmd = am->registerAction(addLibrary,
        Constants::ADDLIBRARY, proFileEditorContext);
    //cmd->setDefaultKeySequence(QKeySequence(Qt::Key_F2));
    connect(addLibrary, SIGNAL(triggered()),
            this, SLOT(addLibrary()));
    contextMenu->addAction(cmd);

    return true;
}

void Qt4ProjectManagerPlugin::extensionsInitialized()
{
    m_qt4ProjectManager->init();
}

void Qt4ProjectManagerPlugin::updateContextMenu(Project *project,
                                                ProjectExplorer::Node *node)
{
    m_qt4ProjectManager->setContextProject(project);
    m_qt4ProjectManager->setContextNode(node);
    m_runQMakeActionContextMenu->setEnabled(false);
    m_buildSubProjectContextMenu->setEnabled(false);
    m_rebuildSubProjectContextMenu->setEnabled(false);
    m_cleanSubProjectContextMenu->setEnabled(false);

    Qt4ProFileNode *proFileNode = qobject_cast<Qt4ProFileNode *>(node);
    if (qobject_cast<Qt4Project *>(project) && proFileNode) {
        m_runQMakeActionContextMenu->setVisible(true);
        m_buildSubProjectContextMenu->setVisible(true);
        m_rebuildSubProjectContextMenu->setVisible(true);
        m_cleanSubProjectContextMenu->setVisible(true);

        if (!m_projectExplorer->buildManager()->isBuilding(project)) {
            m_runQMakeActionContextMenu->setEnabled(true);
            m_buildSubProjectContextMenu->setEnabled(true);
            m_rebuildSubProjectContextMenu->setEnabled(true);
            m_cleanSubProjectContextMenu->setEnabled(true);
        }
    } else {
        m_runQMakeActionContextMenu->setVisible(false);
        m_buildSubProjectContextMenu->setVisible(false);
        m_rebuildSubProjectContextMenu->setVisible(false);
        m_cleanSubProjectContextMenu->setVisible(false);
    }
}

void Qt4ProjectManagerPlugin::currentProjectChanged()
{
    m_runQMakeAction->setEnabled(!m_projectExplorer->buildManager()->isBuilding(m_projectExplorer->currentProject()));
}

void Qt4ProjectManagerPlugin::buildStateChanged(ProjectExplorer::Project *pro)
{
    ProjectExplorer::Project *currentProject = m_projectExplorer->currentProject();
    if (pro == currentProject)
        m_runQMakeAction->setEnabled(!m_projectExplorer->buildManager()->isBuilding(currentProject));
    if (pro == m_qt4ProjectManager->contextProject())
        m_runQMakeActionContextMenu->setEnabled(!m_projectExplorer->buildManager()->isBuilding(pro));
}

void Qt4ProjectManagerPlugin::addLibrary()
{
    Core::EditorManager *em = Core::EditorManager::instance();
    ProFileEditor *editor = qobject_cast<ProFileEditor*>(em->currentEditor()->widget());
    if (editor)
        editor->addLibrary();
}

#ifdef WITH_TESTS
void Qt4ProjectManagerPlugin::testBasicProjectLoading()
{
    QString testDirectory = ExtensionSystem::PluginManager::instance()->testDataDirectory() + "/qt4projectmanager/";
    QString test1 = testDirectory + "test1/test1.pro";
    m_projectExplorer->openProject(test1);
    QVERIFY(!m_projectExplorer->session()->projects().isEmpty());
    Qt4Project *qt4project = qobject_cast<Qt4Project *>(m_projectExplorer->session()->projects().first());
    QVERIFY(qt4project);
    QVERIFY(qt4project->rootProjectNode()->projectType() == ApplicationTemplate);
    QVERIFY(m_projectExplorer->currentProject() != 0);
}
#endif

Q_EXPORT_PLUGIN(Qt4ProjectManagerPlugin)
