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

#ifndef QTUICODEMODELSUPPORT_H
#define QTUICODEMODELSUPPORT_H

#include <cpptools/cppmodelmanagerinterface.h>

#include <QtCore/QDateTime>

namespace Qt4ProjectManager {
class Qt4Project;
namespace Internal {

class Qt4UiCodeModelSupport : public CppTools::AbstractEditorSupport
{
public:
    Qt4UiCodeModelSupport(CppTools::CppModelManagerInterface *modelmanager,
                          Qt4Project *project,
                          const QString &sourceFile,
                          const QString &uiHeaderFile);
    ~Qt4UiCodeModelSupport();
    void setFileName(const QString &name);
    void setSourceName(const QString &name);
    virtual QByteArray contents() const;
    virtual QString fileName() const;
    void updateFromEditor(const QString &formEditorContents);
    void updateFromBuild();
private:
    void init();
    bool runUic(const QString &ui) const;
    Qt4Project *m_project;
    QString m_sourceName;
    QString m_fileName;
    mutable bool m_updateIncludingFiles;
    mutable QByteArray m_contents;
    mutable QDateTime m_cacheTime;
};


} // Internal
} // Qt4ProjectManager
#endif // QTUICODEMODELSUPPORT_H
