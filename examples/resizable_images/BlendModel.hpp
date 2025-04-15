#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtNodes/NodeDelegateModel>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>

#include "PixmapData.hpp"

class BlendModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    BlendModel();
    ~BlendModel() override = default;

    QString caption() const override { return "Blend"; }
    QString name() const override { return "BlendModel"; }
    QString modelName() const { return "Blend"; }

    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex portIndex) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) override;
    QWidget* embeddedWidget() override { return _widget; }

private Q_SLOTS:
    void blend();

private:
    QLabel *_previewLabel;
    QComboBox *_blendModeBox;
    QWidget *_widget;

    QPixmap _pixmap1;
    QPixmap _pixmap2;
    QPixmap _outputPixmap;

    std::shared_ptr<QtNodes::NodeData> _outputData;

    cv::Mat blendImages(const cv::Mat &img1, const cv::Mat &img2, const QString &mode);
};
