#include "node_editor.h"
#include <QVBoxLayout>

NodeEditor::NodeEditor(QWidget* parent)
    : QGraphicsView(parent), m_scene(new QGraphicsScene(this)), m_graph(new NodeGraph(this)) {
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag);
}

NodeEditor::~NodeEditor() {
    delete m_graph;
    delete m_scene;
}

void NodeEditor::addNode(Node* node) {
    m_scene->addItem(node);
    m_graph->addNode(node);
}

void NodeEditor::addConnection(Connection* connection) {
    m_scene->addItem(connection);
    m_graph->addConnection(connection);
}
void NodeEditor::paintEvent(QPaintEvent* event) {
    QGraphicsView::paintEvent(event);
}