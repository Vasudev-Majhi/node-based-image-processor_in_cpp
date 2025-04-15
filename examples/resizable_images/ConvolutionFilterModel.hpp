#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QGridLayout>

#include <QtNodes/NodeDelegateModel>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>

#include "PixmapData.hpp"

class ConvolutionFilterModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    ConvolutionFilterModel();
    ~ConvolutionFilterModel() = default;

public:
    QString caption() const override { return "Convolution Filter"; }
    QString name() const override { return "ConvolutionFilterModel"; }
    QString modelName() const{ return "Convolution Filter"; }

    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType, QtNodes::PortIndex) const override;
    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex) override;
    void setInData(std::shared_ptr<QtNodes::NodeData>, QtNodes::PortIndex) override;
    QWidget *embeddedWidget() override { return _widget; }
    cv::Mat getKernelFromPreset(const QString &preset, int size);

private Q_SLOTS:
    void applyFilter();
    void presetChanged(int index);

private:
    QLabel *_previewLabel;
    QComboBox *_presetBox;
    QPushButton *_applyButton;
    QSpinBox *_kernelSizeBox;
    QWidget *_widget;

    QPixmap _inputPixmap;
    QPixmap _filteredPixmap;

    std::shared_ptr<QtNodes::NodeData> _nodeData;

    cv::Mat applyConvolution(const QPixmap &pixmap, const cv::Mat &kernel);
    cv::Mat getPresetKernel(const QString &preset, int size);
};
