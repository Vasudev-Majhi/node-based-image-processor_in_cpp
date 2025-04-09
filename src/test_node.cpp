// test_node.cpp
#include "test_node.h"
#include <QDebug>

TestNode::TestNode(const QString& name, QGraphicsItem* parent)
    : Node(name, parent) {
}

void TestNode::process() {
    qDebug() << "Processing node:" << m_name;
}