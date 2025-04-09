#pragma once
#include <QGraphicsItem>
#include <QObject>
#include <vector>

class Port;
class Connection;

class Node : public QObject, public QGraphicsItem {
    Q_OBJECT
public:
    Node(QGraphicsItem* parent = nullptr);
    ~Node();

    QRectF boundingRect() const override;
    void paint(QPainter* painter, 
               const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

    void addInputPort(const QString& name);
    void addOutputPort(const QString& name);
    virtual cv::Mat processData(cv::Mat input) = 0;

    std::vector<Port*> inputPorts;
    std::vector<Port*> outputPorts;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    QColor nodeColor;
    int width = 120;
    int height = 80;
};