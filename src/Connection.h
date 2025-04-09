#pragma once
#include <QGraphicsPathItem>

class Port;

class Connection : public QGraphicsPathItem {
public:
    Connection(Port* startPort, Port* endPort, QGraphicsItem* parent = nullptr);
    void updatePath();

    Port* startPort;
    Port* endPort;

private:
    QColor connectionColor = Qt::darkGray;
};