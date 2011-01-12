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

#ifndef DEBUGGER_SNAPSHOTHANDLER_H
#define DEBUGGER_SNAPSHOTHANDLER_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QPointer>

namespace Debugger {

class DebuggerRunControl;
class DebuggerEngine;

namespace Internal {

////////////////////////////////////////////////////////////////////////
//
// SnapshotModel
//
////////////////////////////////////////////////////////////////////////

/*! A model to represent the snapshots in a QTreeView. */
class SnapshotHandler : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit SnapshotHandler();
    ~SnapshotHandler();

    // Called from SnapshotHandler after a new snapshot has been added
    void removeAll();
    QAbstractItemModel *model() { return this; }
    int currentIndex() const { return m_currentIndex; }
    void appendSnapshot(DebuggerRunControl *rc);
    void removeSnapshot(DebuggerRunControl *rc);
    void setCurrentIndex(int index);
    int size() const { return m_snapshots.size(); }
    DebuggerRunControl *at(int i) const;
    QList<DebuggerRunControl*> runControls() const;

private:
    // QAbstractTableModel
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    Q_SLOT void resetModel() { reset(); }
    DebuggerEngine *engineAt(int i) const;
    void removeSnapshot(int index);

    int m_currentIndex;
    QList< QPointer<DebuggerRunControl> > m_snapshots;
    const QVariant m_positionIcon;
    const QVariant m_emptyIcon;
};

} // namespace Internal
} // namespace Debugger

#endif // DEBUGGER_SNAPSHOTHANDLER_H
