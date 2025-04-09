#pragma once
#include <QGraphicsItem>
#include <vector>

class Node;
class Connection;

class Port : public QGraphicsItem {
public:
    enum PortType { Input, Output };
    
    Port(Node* parent, PortType type);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, 
              const QStyleOptionGraphicsItem* option,
              QWidget* widget) override;

    static const int DIAMETER = 16;
    PortType portType;
    std::vector<Connection*> connections;
    Node* parentNode;
};