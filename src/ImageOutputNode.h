#ifndef OUTPUT_NODE_H
#define OUTPUT_NODE_H

#include "node.h"
#include <opencv2/opencv.hpp>
#include <QImage>

class OutputNode : public Node {
    Q_OBJECT
public:
    explicit OutputNode(const QString& name = "Image Output", QGraphicsItem* parent = nullptr);
    void process() override;

private slots:
    void saveImage();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
    Socket*m_inputSocket;
    cv::Mat m_currentImage;
    QImage m_previewImage;
    int m_quality  = 95;
    QString m_format = "PNG";

    void updatePreview();
    QImage cvMatToQImage(const cv::Mat& mat) const;
};

#endif // OUTPUT_NODE_H
