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
#ifndef QTOPTIONSPAGE_H
#define QTOPTIONSPAGE_H

#include <coreplugin/dialogs/ioptionspage.h>

#include <QtCore/QSharedPointer>
#include <QtCore/QFutureInterface>

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE
class QTreeWidgetItem;
QT_END_NAMESPACE

namespace Qt4ProjectManager {

class QtVersion;
typedef QSharedPointer<QtVersion> QSharedPointerQtVersion;

namespace Internal {
namespace Ui {
class QtVersionManager;
}

// A task suitable to be run by QtConcurrent to build the helpers.
// Note that it may outlive the settings page if someone quickly cancels it,
// so, the versions are passed around by QSharedPointer.
class DebuggingHelperBuildTask : public QObject {
    Q_DISABLE_COPY(DebuggingHelperBuildTask)
    Q_OBJECT
public:
    explicit DebuggingHelperBuildTask(const QSharedPointerQtVersion &version);
    virtual ~DebuggingHelperBuildTask();

    void run(QFutureInterface<void> &future);

signals:
    void finished(const QString &versionName, const QString &output);

private:
    QSharedPointerQtVersion m_version;
};

class QtOptionsPageWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(QtOptionsPageWidget)
public:
    QtOptionsPageWidget(QWidget *parent, QList<QtVersion *> versions);
    ~QtOptionsPageWidget();
    QList<QSharedPointerQtVersion> versions() const;
    void finish();
    QString searchKeywords() const;

    virtual bool eventFilter(QObject *o, QEvent *e);

private:
    void showEnvironmentPage(QTreeWidgetItem * item);
    void fixQtVersionName(int index);
    int indexForTreeItem(const QTreeWidgetItem *item) const;
    QTreeWidgetItem *treeItemForIndex(int index) const;
    QtVersion *currentVersion() const;
    int currentIndex() const;
    void updateDebuggingHelperStateLabel(const QtVersion *version = 0);
    QIcon debuggerHelperIconForQtVersion(const QtVersion *version);
    QPixmap debuggerHelperPixmapForQtVersion(const QtVersion *version);

    const QPixmap m_debuggingHelperOkPixmap;
    const QPixmap m_debuggingHelperErrorPixmap;
    const QPixmap m_debuggingHelperIntermediatePixmap;
    const QIcon m_debuggingHelperOkIcon;
    const QIcon m_debuggingHelperErrorIcon;
    const QIcon m_debuggingHelperIntermediateIcon;
    const QString m_specifyNameString;
    const QString m_specifyPathString;

    Internal::Ui::QtVersionManager *m_ui;
    QList<QSharedPointerQtVersion> m_versions; // Passed on to the helper build task, so, use QSharedPointerQtVersion
    int m_defaultVersion;

private slots:
    void versionChanged(QTreeWidgetItem *item, QTreeWidgetItem *old);
    void addQtDir();
    void removeQtDir();
    void updateState();
    void makeMingwVisible(bool visible);
    void makeMSVCVisible(bool visible);
    void makeS60Visible(bool visible);
    void makeDebuggingHelperVisible(bool visible);
    void onQtBrowsed();
    void onMingwBrowsed();
    void updateCurrentQtName();
    void updateCurrentQMakeLocation();
    void updateCurrentMingwDirectory();
    void updateCurrentMwcDirectory();
    void updateCurrentS60SDKDirectory();
    void updateCurrentGcceDirectory();
    void updateCurrentSbsV2Directory();
    void msvcVersionChanged();
    void buildDebuggingHelper();
    void slotShowDebuggingBuildLog();
    void debuggingHelperBuildFinished(const QString &versionName, const QString &output);

private:
    void showDebuggingBuildLog(const QTreeWidgetItem *currentItem);
};

class QtOptionsPage : public Core::IOptionsPage
{
    Q_OBJECT
public:
    QtOptionsPage();
    QString id() const;
    QString displayName() const;
    QString category() const;
    QString displayCategory() const;
    QIcon categoryIcon() const;
    QWidget *createPage(QWidget *parent);
    void apply();
    void finish() { }
    virtual bool matches(const QString &) const;

private:
    QtOptionsPageWidget *m_widget;
    QString m_searchKeywords;
};

} //namespace Internal
} //namespace Qt4ProjectManager


#endif // QTOPTIONSPAGE_H
