#include <QtNodes/DataFlowGraphModel>
#include <QtNodes/DataFlowGraphicsScene>
#include <QtNodes/GraphicsView>
#include <QtNodes/NodeData>
#include <QtNodes/NodeDelegateModelRegistry>

#include <QtGui/QScreen>
#include <QtWidgets/QApplication>

#include "ImageLoaderModel.hpp"
#include "ImageShowModel.hpp"
#include "BrightnessContrastModel.hpp"
#include "GaussianBlurModel.hpp"
#include "ThresholdModel.hpp"
#include "EdgeDetectionModel.hpp"
#include "BlendModel.hpp"
#include "NoiseGenerationModel.hpp"
#include "ConvolutionFilterModel.hpp"

using QtNodes::ConnectionStyle;
using QtNodes::DataFlowGraphicsScene;
using QtNodes::DataFlowGraphModel;
using QtNodes::GraphicsView;
using QtNodes::NodeDelegateModelRegistry;

static std::shared_ptr<NodeDelegateModelRegistry> registerDataModels()
{
    auto ret = std::make_shared<NodeDelegateModelRegistry>();
    ret->registerModel<ImageShowModel>();

    ret->registerModel<ImageLoaderModel>();

    ret->registerModel<BrightnessContrastModel>();

    ret->registerModel<GaussianBlurModel>();

    ret->registerModel<ThresholdModel>();

    ret->registerModel<EdgeDetectionModel>();

    ret->registerModel<BlendModel>();

    ret->registerModel<NoiseGenerationModel>();

    ret->registerModel<ConvolutionFilterModel>();




    return ret;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    std::shared_ptr<NodeDelegateModelRegistry> registry = registerDataModels();

    DataFlowGraphModel dataFlowGraphModel(registry);

    DataFlowGraphicsScene scene(dataFlowGraphModel);

    GraphicsView view(&scene);

    view.setWindowTitle("Mixar Final-v3 Project");
    view.resize(800, 600);
    // Center window.
    view.move(QApplication::primaryScreen()->availableGeometry().center() - view.rect().center());
    view.show();

    return app.exec();
}
