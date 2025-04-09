#pragma once
#include <QGraphicsScene>

class Port;

class NodeGraphScene : public QGraphicsScene {
    Q_OBJECT
public:
    NodeGraphScene(QObject* parent = nullptr);
    
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    Port* tempStartPort = nullptr;
    QGraphicsPathItem* tempConnection = nullptr;
};