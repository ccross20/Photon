#include "canvasglobalsnode.h"

namespace photon {

const QByteArray CanvasGlobalsNode::WidthParam = "width";
const QByteArray CanvasGlobalsNode::HeightParam = "height";
const QByteArray CanvasGlobalsNode::TimeParam = "relativeTime";
const QByteArray CanvasGlobalsNode::GlobalTimeParam = "globalTime";

keira::NodeInformation CanvasGlobalsNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasGlobalsNode;});
    toReturn.name = "Canvas Globals";
    toReturn.nodeId = "photon.canvas.globals";
    toReturn.categories = {"Canvas"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

CanvasGlobalsNode::CanvasGlobalsNode() : keira::Node("photon.canvas.globals")
{
    setName("Globals");
}

void CanvasGlobalsNode::createParameters()
{
    m_widthParam = new keira::IntegerParameter(WidthParam, "Width", 256, keira::AllowMultipleOutput);
    m_heightParam = new keira::IntegerParameter(HeightParam, "Height", 256, keira::AllowMultipleOutput);
    m_timeParam = new keira::DecimalParameter(TimeParam, "Relative Time", 0, keira::AllowMultipleOutput);
    m_globalTimeParam = new keira::DecimalParameter(GlobalTimeParam, "Global Time", 0, keira::AllowMultipleOutput);

    addParameter(m_widthParam);
    addParameter(m_heightParam);
    addParameter(m_timeParam);
    addParameter(m_globalTimeParam);
}

} // namespace photon
