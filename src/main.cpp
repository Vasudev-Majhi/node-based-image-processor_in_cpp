// #include <QApplication>
// #include "node_editor.h"
// #include "test_node.h"
// #include "socket.h"
// #include "connection.h"

// int main(int argc, char** argv) {
//     QApplication app(argc, argv);

//     NodeEditor editor;
//     editor.setWindowTitle("Node-Based Image Editor");
//     editor.resize(800, 600);

//     // Create nodes
//     TestNode* node1 = new TestNode("Source Node");
//     TestNode* node2 = new TestNode("Filter Node");
//     TestNode* node3 = new TestNode("Output Node");

//     // Set node positions
//     node1->setPos(-150, 0);
//     node2->setPos(0, 0);
//     node3->setPos(150, 0);

//     // Create sockets
//     Socket* out1 = new Socket(SocketType::OUTPUT, node1);
//     Socket* in2 = new Socket(SocketType::INPUT, node2);
//     Socket* out2 = new Socket(SocketType::OUTPUT, node2);
//     Socket* in3 = new Socket(SocketType::INPUT, node3);

//     // Add sockets to nodes
//     node1->addSocket(out1);
//     node2->addSocket(in2);
//     node2->addSocket(out2);
//     node3->addSocket(in3);

//     // Create connections
//     Connection* conn1 = new Connection(out1, in2);
//     Connection* conn2 = new Connection(out2, in3);

//     // Add nodes and connections to editor
//     editor.addNode(node1);
//     editor.addNode(node2);
//     editor.addNode(node3);
//     editor.addConnection(conn1);
//     editor.addConnection(conn2);

//     editor.show();

//     return app.exec();
// }

// #include <QApplication>
// #include "node_editor.h"
// #include "node.h"
// #include "socket.h"
// #include <QDebug>

// class TestNode : public Node {
// public:
//     TestNode(const QString& name) : Node(name) {
//         // Input socket on the left
//         Socket* input = new Socket(INPUT, this);
//         input->setPos(-90, 0); // Position within node's bounding rect
//         addSocket(input);

//         // Output socket on the right
//         Socket* output = new Socket(OUTPUT, this);
//         output->setPos(90, 0);
//         addSocket(output);
//     }

//     void process() override {
//         qDebug() << "Processing" << m_name;
//         // Add actual processing logic here
//     }
// };

// int main(int argc, char** argv) {
//     QApplication app(argc, argv);

//     NodeEditor editor;
//     editor.setWindowTitle("Node-Based Image Editor");
//     editor.resize(800, 600);

//     // Create and position nodes
//     TestNode* node1 = new TestNode("Node 1");
//     node1->setPos(-200, 0);

//     TestNode* node2 = new TestNode("Node 2");
//     node2->setPos(200, 0);

//     // Add nodes to the editor
//     editor.addNode(node1);
//     editor.addNode(node2);

//     // Create connection between node1's output and node2's input
//     Socket* fromSocket = node1->sockets().at(1); // Output socket
//     Socket* toSocket = node2->sockets().at(0);   // Input socket

//     Connection* connection = new Connection(fromSocket, toSocket);
//     editor.addConnection(connection);

//     // Optional: Adjust the view to fit the scene
//     editor.fitInView(editor.scene()->sceneRect(), Qt::KeepAspectRatio);

//     editor.show();
//     return app.exec();
// }

// main.cpp
#include <QApplication>
#include <QTimer>
#include "node_editor.h"
#include "socket.h"
#include "connection.h"

// 1) Include your new nodes
#include "ImageInputNode.h"
#include "ImageOutputNode.h"     

#include <opencv2/opencv.hpp>
#include <QMetaType>
// #include "MetaTypes.h"
Q_DECLARE_METATYPE(cv::Mat)

int main(int argc, char** argv) {
     // 2) Allow cv::Mat in QVariant
    qRegisterMetaType<cv::Mat>("cv::Mat");
    QApplication app(argc, argv);

   
  

    // 3) Create the editor
    NodeEditor editor;
    editor.setWindowTitle("Node‑Based Image Editor");
    editor.resize(800, 600);

    // 4) Instantiate an ImageInputNode
    ImageInputNode* inputNode = new ImageInputNode("Load Image");
    inputNode->setPos(-200, 0);
    editor.addNode(inputNode);

    // 5) (Optional) Instantiate an OutputNode and connect it
    OutputNode* outputNode = new OutputNode("Save/Preview");
    outputNode->setPos(200, 0);
    editor.addNode(outputNode);

    Socket* outSock = inputNode->sockets().at(0);  // OUTPUT socket
    Socket* inSock  = outputNode->sockets().at(0); // INPUT socket
    Connection* conn = new Connection(outSock, inSock);
    editor.addConnection(conn);

    // 6) Fit view
    editor.fitInView(editor.scene()->sceneRect(), Qt::KeepAspectRatio);
    editor.show();

    // 7) Delay a tiny bit then trigger the file‑open dialog
    //    (so that the editor window is visible first)
    QTimer::singleShot(200, [inputNode]() {
        inputNode->loadImage();
    });

    return app.exec();
}
