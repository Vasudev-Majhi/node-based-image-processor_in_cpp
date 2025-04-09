#ifndef NODE_EDITOR_H
#define NODE_EDITOR_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include "node_graph.h"

class NodeEditor : public QGraphicsView {
    Q_OBJECT
public:
    NodeEditor(QWidget* parent = nullptr);
    ~NodeEditor();

    void addNode(Node* node);
    void addConnection(Connection* connection);
    
protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QGraphicsScene* m_scene;
    NodeGraph* m_graph;
};

#endif // NODE_EDITOR_H