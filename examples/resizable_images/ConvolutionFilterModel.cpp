#include "ConvolutionFilterModel.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>

#include <QImage>
#include <QVBoxLayout>
#include <QHBoxLayout>

ConvolutionFilterModel::ConvolutionFilterModel()
{
    _widget = new QWidget;
    _previewLabel = new QLabel("Preview");
    _presetBox = new QComboBox;
    _kernelSizeBox = new QSpinBox;
    _applyButton = new QPushButton("Apply");

    _presetBox->addItems({ "Sharpen", "Emboss", "Edge Enhance" });
    _kernelSizeBox->setRange(3, 5);
    _kernelSizeBox->setSingleStep(2);
    _kernelSizeBox->setValue(3);

    auto *layout = new QVBoxLayout(_widget);
    layout->addWidget(_previewLabel);

    auto *controlLayout = new QHBoxLayout;
    controlLayout->addWidget(_presetBox);
    controlLayout->addWidget(_kernelSizeBox);
    controlLayout->addWidget(_applyButton);
    layout->addLayout(controlLayout);

    connect(_applyButton, &QPushButton::clicked, this, &ConvolutionFilterModel::applyFilter);
    connect(_presetBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConvolutionFilterModel::presetChanged);
}

unsigned int ConvolutionFilterModel::nPorts(QtNodes::PortType portType) const
{
    return (portType == QtNodes::PortType::In) ? 1 : 1;
}

QtNodes::NodeDataType ConvolutionFilterModel::dataType(QtNodes::PortType, QtNodes::PortIndex) const
{
    return PixmapData().type();
}

std::shared_ptr<QtNodes::NodeData> ConvolutionFilterModel::outData(QtNodes::PortIndex)
{
    return std::make_shared<PixmapData>(_filteredPixmap);
}

void ConvolutionFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex)
{
    _nodeData = nodeData;

    if (auto d = std::dynamic_pointer_cast<PixmapData>(nodeData)) {
        _inputPixmap = d->pixmap();
        _previewLabel->setPixmap(_inputPixmap.scaled(200, 200, Qt::KeepAspectRatio));
        applyFilter();
    } else {
        _inputPixmap = QPixmap();
        _previewLabel->clear();
    }

    Q_EMIT dataUpdated(0);
}

cv::Mat ConvolutionFilterModel::applyConvolution(const QPixmap &pixmap, const cv::Mat &kernel)
{
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB888);
    cv::Mat src(image.height(), image.width(), CV_8UC3, const_cast<uchar *>(image.bits()), image.bytesPerLine());
    cv::Mat dst;
    cv::filter2D(src, dst, -1, kernel);
    return dst;
}
cv::Mat ConvolutionFilterModel::getPresetKernel(const QString &preset, int size)
{
    if (preset == "Sharpen")
    {
        if (size == 3)
        {
            return (cv::Mat_<float>(3, 3) << 
                    0, -1,  0,
                   -1,  5, -1,
                    0, -1,  0);
        }
        else
        {
            return cv::Mat::eye(size, size, CV_32F);
        }
    }
    else if (preset == "Emboss")
    {
        if (size == 3)
        {
            return (cv::Mat_<float>(3, 3) <<
                   -2, -1, 0,
                   -1,  1, 1,
                    0,  1, 2);
        }
        else
        {
            return cv::Mat::eye(size, size, CV_32F);
        }
    }
    else if (preset == "Edge Enhance")
    {
        if (size == 3)
        {
            return (cv::Mat_<float>(3, 3) <<
                    0,  0,  0,
                   -1,  1,  0,
                    0,  0,  0);
        }
        else
        {
            return cv::Mat::eye(size, size, CV_32F);
        }
    }

    return cv::Mat::eye(size, size, CV_32F);
}


cv::Mat ConvolutionFilterModel::getKernelFromPreset(const QString &preset, int size)
{
    if (preset == "Sharpen") {
        if (size == 3) {
            cv::Mat_<float> k(3, 3);
            k <<  0, -1,  0,
                -1,  5, -1,
                 0, -1,  0;
            return k;
        }
    }
    else if (preset == "Emboss") {
        if (size == 3) {
            cv::Mat_<float> k(3, 3);
            k << -2, -1,  0,
                -1,  1,  1,
                 0,  1,  2;
            return k;
        }
    }
    else if (preset == "Edge Enhance") {
        if (size == 3) {
            cv::Mat_<float> k(3, 3);
            k <<  0,  0,  0,
                 -1,  1,  0,
                  0,  0,  0;
            return k;
        }
    }

    // Default: identity kernel
    cv::Mat_<float> identity(size, size, 0.0f);
    identity(size / 2, size / 2) = 1.0f;
    return identity;
}


void ConvolutionFilterModel::applyFilter()
{
    if (_inputPixmap.isNull()) return;

    QString preset = _presetBox->currentText();
    int kernelSize = _kernelSizeBox->value();

    cv::Mat kernel = getPresetKernel(preset, kernelSize);
    cv::Mat result = applyConvolution(_inputPixmap, kernel);

    QImage outputImg(result.data, result.cols, result.rows, result.step, QImage::Format_RGB888);
    _filteredPixmap = QPixmap::fromImage(outputImg.rgbSwapped());

    _previewLabel->setPixmap(_filteredPixmap.scaled(200, 200, Qt::KeepAspectRatio));

    Q_EMIT dataUpdated(0);
}

void ConvolutionFilterModel::presetChanged(int)
{
    applyFilter();
}
