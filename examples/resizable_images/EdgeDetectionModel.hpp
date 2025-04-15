#pragma once

#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <QtNodes/NodeDelegateModel>
#include "PixmapData.hpp"

class EdgeDetectionModel : public QtNodes::NodeDelegateModel {
    Q_OBJECT

public:
    EdgeDetectionModel();
    ~EdgeDetectionModel() = default;

    QString caption() const override { return QString("Edge Detection"); }
    QString name() const override { return QString("EdgeDetectionModel"); }
    QString modelName() const { return QString("Edge Detection"); }

    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex port) override;

    QWidget* embeddedWidget() override { return _widget; }
    bool resizable() const override { return true; }

private:
    void processImage();
    void updateDisplay(const QPixmap& pixmap);

private:
    QLabel* _previewLabel;
    QWidget* _widget;
    QComboBox* _methodCombo;
    QSlider* _threshold1Slider;
    QSlider* _threshold2Slider;
    QSlider* _kernelSizeSlider;
    QCheckBox* _overlayCheckBox;

    QPixmap _originalPixmap;
    std::shared_ptr<QtNodes::NodeData> _output;
};
