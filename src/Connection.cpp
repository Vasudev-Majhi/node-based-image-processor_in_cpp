#include "Connection.h"
#include "Port.h"
#include <QPainter>

Connection::Connection(Port* start, Port* end, QGraphicsItem* parent)
    : QGraphicsPathItem(parent), startPort(start), endPort(end) {
    setPen(QPen(Qt::darkGray, 2));
    setBrush(Qt::NoBrush);
    setZValue(-1);
    updatePath();
}

void Connection::updatePath() {
    QPainterPath path;
    
    if (startPort && endPort) {
        QPointF startPos = startPort->scenePos();
        QPointF endPos = endPort->scenePos();
        
        path.moveTo(startPos);
        qreal dx = endPos.x() - startPos.x();
        qreal dy = endPos.y() - startPos.y();
        
        QPointF ctr1(startPos.x() + dx * 0.25, startPos.y() + dy * 0.1);
        QPointF ctr2(startPos.x() + dx * 0.75, startPos.y() + dy * 0.9);
        
        path.cubicTo(ctr1, ctr2, endPos);
    }
    
    setPath(path);
}