#include "pixelglobalsnode.h"

namespace photon {

const QByteArray PixelGlobalsNode::FixtureParam = "fixture";
const QByteArray PixelGlobalsNode::FixtureIndexParam = "fixtureIndex";
const QByteArray PixelGlobalsNode::PixelIndexParam = "pixelIndex";
const QByteArray PixelGlobalsNode::PixelGlobalIndexParam = "pixelGlobalIndex";
const QByteArray PixelGlobalsNode::PixelTotalParam = "pixelTotal";
const QByteArray PixelGlobalsNode::TimeParam = "relativeTime";
const QByteArray PixelGlobalsNode::GlobalTimeParam = "globalTime";
const QByteArray PixelGlobalsNode::TimeOffsetParam = "timeOffset";


keira::NodeInformation PixelGlobalsNode::info()
{
    keira::NodeInformation toReturn([](){return new PixelGlobalsNode;});
    toReturn.name = "Pixel Globals";
    toReturn.nodeId = "photon.pixel.globals";
    toReturn.categories = {"Pixel"};
    toReturn.graphs = QByteArrayList{"pixel"};

    return toReturn;
}


PixelGlobalsNode::PixelGlobalsNode() : keira::Node("photon.pixel.globals"){
    setName("Globals");
}

void PixelGlobalsNode::createParameters()
{

    m_fixtureParam = new FixtureParameter(FixtureParam,"Fixture","",keira::AllowMultipleOutput);
    m_timeParam = new keira::DecimalParameter(TimeParam, "Relative Time",0,keira::AllowMultipleOutput);
    m_globalTimeParam = new keira::DecimalParameter(GlobalTimeParam, "Global Time",0,keira::AllowMultipleOutput);
    m_timeOffsetParam = new keira::DecimalParameter(TimeOffsetParam, "Time Offset",0,keira::AllowMultipleOutput);
    m_fixtureIndexParam = new keira::IntegerParameter(FixtureIndexParam, "Fixture Index",0,keira::AllowMultipleOutput);
    m_pixelIndexParam = new keira::IntegerParameter(PixelIndexParam, "Pixel Index",0,keira::AllowMultipleOutput);
    m_pixelGlobalIndexParam = new keira::IntegerParameter(PixelGlobalIndexParam, "Pixel Global Index",0,keira::AllowMultipleOutput);
    m_pixelTotalParam = new keira::IntegerParameter(PixelTotalParam, "Total Pixels",0,keira::AllowMultipleOutput);

    addParameter(m_pixelIndexParam);
    addParameter(m_pixelGlobalIndexParam);
    addParameter(m_pixelTotalParam);
    addParameter(m_fixtureParam);
    addParameter(m_fixtureIndexParam);
    addParameter(m_timeParam);
    addParameter(m_globalTimeParam);
    addParameter(m_timeOffsetParam);

}

} // namespace photon
