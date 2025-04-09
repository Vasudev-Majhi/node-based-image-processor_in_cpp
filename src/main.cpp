#include <QApplication>
#include <QMainWindow>
#include <QGraphicsView>
#include "NodeGraphScene.h"
#include "ImageLoaderNode.h"
#include "ImageViewerNode.h"

class MainWindow : public QMainWindow {
public:
    MainWindow() {
        scene = new NodeGraphScene(this);
        view = new QGraphicsView();
        view->setScene(scene);
        setCentralWidget(view);

        ImageLoaderNode* loader = new ImageLoaderNode();
        ImageViewerNode* viewer = new ImageViewerNode();
        
        scene->addItem(loader);
        scene->addItem(viewer);
        loader->setPos(-200, 0);
        viewer->setPos(200, 0);
    }

private:
    QGraphicsView* view;
    NodeGraphScene* scene;
};

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.resize(800, 600);
    w.show();
    return a.exec();
}