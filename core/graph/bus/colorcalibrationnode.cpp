#include "colorcalibrationnode.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "graph/parameter/fixtureparameter.h"
#include "model/parameter/booleanparameter.h"
#include "graph/bus/busgraph.h"
#include "fixture/fixture.h"
#include "fixture/fixturecollection.h"
#include "fixture/capability/colorcapability.h"

namespace photon {

const QByteArray ColorCalibrationNode::InputDMX = "dmxInput";
const QByteArray ColorCalibrationNode::OutputDMX = "dmxOutput";
const QByteArray ColorCalibrationNode::FixtureParam = "fixture";
const QByteArray ColorCalibrationNode::EnabledParam = "enabled";

class ColorCalibrationNode::Impl
{
public:
    DMXMatrixParameter *dmxInParam;
    DMXMatrixParameter *dmxOutParam;
    FixtureParameter *fixtureParam;
    keira::BooleanParameter *enabledParam;
    QMap<CapabilityType, double> channelPercents;
};

keira::NodeInformation ColorCalibrationNode::info()
{
    keira::NodeInformation toReturn([](){return new ColorCalibrationNode;});
    toReturn.name = "Color Calibration";
    toReturn.nodeId = "photon.bus.color-calibration";
    toReturn.graphs = QByteArrayList{BusGraph::BusGraphId};

    return toReturn;
}

ColorCalibrationNode::ColorCalibrationNode() : keira::Node("photon.bus.color-calibration"), m_impl(new Impl)
{
    setName("Color Calibration");
}

ColorCalibrationNode::~ColorCalibrationNode()
{
    delete m_impl;
}

void ColorCalibrationNode::createParameters()
{
    m_impl->dmxInParam = new DMXMatrixParameter(InputDMX, "DMX Input", DMXMatrix());
    addParameter(m_impl->dmxInParam);

    m_impl->fixtureParam = new FixtureParameter(FixtureParam, "Fixture", "");
    addParameter(m_impl->fixtureParam);

    m_impl->enabledParam = new keira::BooleanParameter(EnabledParam, "Preview", false);
    addParameter(m_impl->enabledParam);

    m_impl->dmxOutParam = new DMXMatrixParameter(OutputDMX, "DMX Output", DMXMatrix(), keira::AllowMultipleOutput);
    addParameter(m_impl->dmxOutParam);
}

void ColorCalibrationNode::setTargetFixture(const QByteArray &t_fixtureId)
{
    m_impl->fixtureParam->setValue(t_fixtureId);
}

void ColorCalibrationNode::setPreviewEnabled(bool t_enabled)
{
    m_impl->enabledParam->setValue(t_enabled);
}

bool ColorCalibrationNode::isPreviewEnabled() const
{
    return m_impl->enabledParam->value().toBool();
}

void ColorCalibrationNode::setChannelPercent(CapabilityType t_type, double t_percent)
{
    m_impl->channelPercents.insert(t_type, t_percent);
}

void ColorCalibrationNode::clearChannelPercents()
{
    m_impl->channelPercents.clear();
}

void ColorCalibrationNode::evaluate(keira::EvaluationContext *) const
{
    DMXMatrix matrix = m_impl->dmxInParam->value().value<DMXMatrix>();

    if(m_impl->enabledParam->value().toBool())
    {
        Fixture *fixture = FixtureCollection::fixtureById(m_impl->fixtureParam->value().toByteArray());
        if(fixture)
        {
            for(int i = 0; i < fixture->colorCount(); ++i)
                fixture->colorAtIndex(i)->setChannelPercents(m_impl->channelPercents, matrix);
        }
    }

    m_impl->dmxOutParam->setValue(matrix);
}

} // namespace photon
