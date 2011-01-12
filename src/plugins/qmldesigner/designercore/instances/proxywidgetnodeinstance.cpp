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

#include "proxywidgetnodeinstance.h"

#include <QGraphicsProxyWidget>
#include <private/qdeclarativemetatype_p.h>

namespace QmlDesigner {
namespace Internal {


ProxyWidgetNodeInstance::ProxyWidgetNodeInstance(QGraphicsProxyWidget *widget)
  : GraphicsWidgetNodeInstance(widget)
{
}


ProxyWidgetNodeInstance::Pointer ProxyWidgetNodeInstance::create(const QString &typeName)
{
    QObject *object = QDeclarativeMetaType::qmlType(typeName.toLatin1(), 4, 7)->create();
    Q_ASSERT(object);
    if (object == 0)
        return Pointer();

    QGraphicsProxyWidget* widget = qobject_cast<QGraphicsProxyWidget*>(object);
    Q_ASSERT(widget);
    if (widget == 0)
        return Pointer();

    return Pointer(new ProxyWidgetNodeInstance(widget));
}

QGraphicsProxyWidget* ProxyWidgetNodeInstance::proxyWidget() const
{
    QGraphicsProxyWidget* proxyWidget = qobject_cast<QGraphicsProxyWidget*>(graphicsWidget());
    Q_ASSERT(proxyWidget);

    return proxyWidget;
}


void ProxyWidgetNodeInstance::setWidget(QWidget *widget)
{
    proxyWidget()->setWidget(widget);
}

bool ProxyWidgetNodeInstance::isProxyWidget() const
{
    return true;
}


}
}
