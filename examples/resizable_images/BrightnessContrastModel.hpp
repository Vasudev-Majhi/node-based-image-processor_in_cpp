#pragma once

#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <QtNodes/NodeDelegateModel>

#include "PixmapData.hpp"

class BrightnessContrastModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    BrightnessContrastModel();
    ~BrightnessContrastModel() override = default;

    QString caption() const override;
    QString name() const override;
    QString modelName() const;

    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> data, QtNodes::PortIndex port) override;

    QWidget *embeddedWidget() override;
    bool resizable() const override { return true; }

private Q_SLOTS:
    void onValueChanged();

private:
    QPixmap _originalPixmap;
    QPixmap _processedPixmap;

    QWidget *_widget;
    QLabel *_previewLabel;
    QSlider *_brightnessSlider;
    QSlider *_contrastSlider;

    std::shared_ptr<QtNodes::NodeData> _nodeData;
};
