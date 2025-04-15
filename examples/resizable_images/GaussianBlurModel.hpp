#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QSlider>

#include <QtNodes/NodeDelegateModel>

#include "PixmapData.hpp"

using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDelegateModel;
using QtNodes::PortIndex;
using QtNodes::PortType;

class GaussianBlurModel : public NodeDelegateModel
{
    Q_OBJECT

public:
    GaussianBlurModel();
    ~GaussianBlurModel() override = default;

public:
    QString caption() const override { return QString("Gaussian Blur"); }
    QString name() const override { return QString("GaussianBlurModel"); }
    QString modelName() const { return QString("Gaussian Blur"); }

    unsigned int nPorts(PortType portType) const override;
    NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

    std::shared_ptr<NodeData> outData(PortIndex port) override;
    void setInData(std::shared_ptr<NodeData> nodeData, PortIndex port) override;

    QWidget *embeddedWidget() override { return _widget; }
    bool resizable() const override { return true; }

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    void applyGaussianBlur(const QPixmap &input);

private:
    QLabel *_label;
    QSlider *_slider;
    QWidget *_widget;
    QVBoxLayout *_layout;

    QPixmap _originalPixmap;
    QPixmap _blurredPixmap;

    int _blurRadius = 5;
};
