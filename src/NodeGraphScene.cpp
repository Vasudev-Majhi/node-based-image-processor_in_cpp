#include "NodeGraphScene.h"
#include "Port.h"
#include "Connection.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>

NodeGraphScene::NodeGraphScene(QObject* parent) 
    : QGraphicsScene(parent) {}

void NodeGraphScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
    
    if(Port* port = dynamic_cast<Port*>(item)) {
        tempStartPort = port;
        tempConnection = new QGraphicsPathItem();
        tempConnection->setPen(QPen(Qt::gray, 2));
        addItem(tempConnection);
    }
    
    QGraphicsScene::mousePressEvent(event);
}

void NodeGraphScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if(tempConnection) {
        QPainterPath path;
        path.moveTo(tempStartPort->scenePos());
        path.lineTo(event->scenePos());
        tempConnection->setPath(path);
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void NodeGraphScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if(tempConnection) {
        QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
        
        if(Port* endPort = dynamic_cast<Port*>(item)) {
            if(tempStartPort->portType != endPort->portType && 
               tempStartPort->parentNode != endPort->parentNode) {
                Connection* conn = new Connection(tempStartPort, endPort);
                addItem(conn);
                conn->updatePath();
                
                tempStartPort->connections.push_back(conn);
                endPort->connections.push_back(conn);
            }
        }
        
        removeItem(tempConnection);
        delete tempConnection;
        tempConnection = nullptr;
        tempStartPort = nullptr;
    }
    
    QGraphicsScene::mouseReleaseEvent(event);
}