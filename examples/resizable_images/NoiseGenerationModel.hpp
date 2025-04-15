#pragma once

#include <QtNodes/NodeDelegateModel>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>

#include "PixmapData.hpp"

class NoiseGenerationModel : public QtNodes::NodeDelegateModel
{
    Q_OBJECT

public:
    NoiseGenerationModel();
    ~NoiseGenerationModel() override = default;

    QString caption() const override { return "Noise Generator"; }
    QString name() const override { return "NoiseGenerationModel"; }
    QString modelName() const  { return "Noise Generator"; }

    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;
    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData>, QtNodes::PortIndex) override {}

    QWidget* embeddedWidget() override { return _widget; }
    bool resizable() const override { return true; }

private Q_SLOTS:
    void generateNoise();

private:
    void updatePreview();
    QImage generatePerlinNoise(int width, int height, float scale, int octaves, float persistence);
    QImage generateMockNoise(int width, int height); // for Simplex/Worley

private:
    QWidget* _widget = nullptr;
    QLabel* _previewLabel = nullptr;

    QComboBox* _noiseTypeCombo = nullptr;
    QSlider* _scaleSlider = nullptr;
    QSlider* _octaveSlider = nullptr;
    QSlider* _persistenceSlider = nullptr;
    QCheckBox* _displacementCheck = nullptr;

    QPixmap _pixmap;
};
