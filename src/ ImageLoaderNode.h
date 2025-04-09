#pragma once
#include "Node.h"

class ImageLoaderNode : public Node {
    Q_OBJECT
public:
    ImageLoaderNode(QGraphicsItem* parent = nullptr);
    cv::Mat processData(cv::Mat input) override;

private:
    QString filePath;
    cv::Mat loadedImage;
};