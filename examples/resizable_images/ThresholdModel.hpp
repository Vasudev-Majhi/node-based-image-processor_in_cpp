#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>

#include <QtNodes/NodeDelegateModel>

#include "PixmapData.hpp"

using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDelegateModel;
using QtNodes::PortIndex;
using QtNodes::PortType;

class ThresholdModel : public NodeDelegateModel
{
    Q_OBJECT

public:
    ThresholdModel();
    ~ThresholdModel() override = default;

public:
    QString caption() const override { return QString("Threshold"); }
    QString name() const override { return QString("ThresholdModel"); }
    QString modelName() const { return QString("Threshold"); }

    unsigned int nPorts(PortType portType) const override;
    NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

    std::shared_ptr<NodeData> outData(PortIndex port) override;
    void setInData(std::shared_ptr<NodeData> nodeData, PortIndex port) override;

    QWidget *embeddedWidget() override { return _widget; }
    bool resizable() const override { return true; }

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    void applyThreshold(const QPixmap &input);

private:
    QLabel *_label;
    QSlider *_slider;
    QWidget *_widget;
    QVBoxLayout *_layout;

    QPixmap _originalPixmap;
    QPixmap _thresholdPixmap;

    int _thresholdValue = 128;
};
