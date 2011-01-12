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

#include "graphicsscenenodeinstance.h"

#include "graphicsviewnodeinstance.h"

#include <invalidnodeinstanceexception.h>
#include <propertymetainfo.h>

namespace QmlDesigner {
namespace Internal {

GraphicsSceneNodeInstance::GraphicsSceneNodeInstance(QGraphicsScene *scene)
   :ObjectNodeInstance(scene)
{
}

GraphicsSceneNodeInstance::~GraphicsSceneNodeInstance()
{
}

GraphicsSceneNodeInstance::Pointer GraphicsSceneNodeInstance::create(const NodeMetaInfo &nodeMetaInfo, QDeclarativeContext *context, QObject  *objectToBeWrapped)
{
    QObject *object = 0;
    if (objectToBeWrapped)
        object = objectToBeWrapped;
    else
        object = createObject(nodeMetaInfo, context);

    QGraphicsScene* scene = qobject_cast<QGraphicsScene*>(object);
    if (scene == 0)
        throw InvalidNodeInstanceException(__LINE__, __FUNCTION__, __FILE__);

    Pointer instance(new GraphicsSceneNodeInstance(scene));

    if (objectToBeWrapped)
        instance->setDeleteHeldInstance(false); // the object isn't owned

    instance->populateResetValueHash();

    return instance;
}

void GraphicsSceneNodeInstance::paint(QPainter *) const
{
    Q_ASSERT(graphicsScene());
}

bool GraphicsSceneNodeInstance::isTopLevel() const
{
    Q_ASSERT(graphicsScene());
    return graphicsScene()->views().isEmpty();
}


void GraphicsSceneNodeInstance::addItem(QGraphicsItem *item)
{
    graphicsScene()->addItem(item);
}

bool GraphicsSceneNodeInstance::isGraphicsScene() const
{
    return true;
}

QRectF GraphicsSceneNodeInstance::boundingRect() const
{
    return graphicsScene()->sceneRect();
}

QPointF GraphicsSceneNodeInstance::position() const
{
    return graphicsScene()->sceneRect().topLeft();
}

QSizeF GraphicsSceneNodeInstance::size() const
{
    return graphicsScene()->sceneRect().size();
}

QGraphicsScene *GraphicsSceneNodeInstance::graphicsScene() const
{
    Q_ASSERT(qobject_cast<QGraphicsScene*>(object()));
    return static_cast<QGraphicsScene*>(object());
}

bool GraphicsSceneNodeInstance::isVisible() const
{
    return false;
}

void GraphicsSceneNodeInstance::setVisible(bool /*isVisible*/)
{

}


}
}
