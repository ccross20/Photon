#include <QColor>
#include "identifyfixturenode.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "graph/parameter/fixtureparameter.h"
#include "model/parameter/booleanparameter.h"
#include "graph/bus/busgraph.h"
#include "fixture/fixture.h"
#include "fixture/fixturecollection.h"
#include "fixture/fixturechannel.h"
#include "fixture/capability/dimmercapability.h"
#include "fixture/capability/shutterstrobecapability.h"
#include "fixture/capability/colorcapability.h"

namespace photon {

const QByteArray IdentifyFixtureNode::InputDMX = "dmxInput";
const QByteArray IdentifyFixtureNode::OutputDMX = "dmxOutput";
const QByteArray IdentifyFixtureNode::FixtureParam = "fixture";
const QByteArray IdentifyFixtureNode::EnabledParam = "enabled";

class IdentifyFixtureNode::Impl
{
public:
    DMXMatrixParameter *dmxInParam;
    DMXMatrixParameter *dmxOutParam;
    FixtureParameter *fixtureParam;
    keira::BooleanParameter *enabledParam;
};

keira::NodeInformation IdentifyFixtureNode::info()
{
    keira::NodeInformation toReturn([](){return new IdentifyFixtureNode;});
    toReturn.name = "Identify";
    toReturn.nodeId = "photon.bus.identify-fixture";
    toReturn.graphs = QByteArrayList{BusGraph::BusGraphId};

    return toReturn;
}

IdentifyFixtureNode::IdentifyFixtureNode() : keira::Node("photon.bus.identify-fixture"), m_impl(new Impl)
{
    setName("Identify");
}

IdentifyFixtureNode::~IdentifyFixtureNode()
{
    delete m_impl;
}

void IdentifyFixtureNode::createParameters()
{
    m_impl->dmxInParam = new DMXMatrixParameter(InputDMX, "DMX Input", DMXMatrix());
    addParameter(m_impl->dmxInParam);

    m_impl->fixtureParam = new FixtureParameter(FixtureParam, "Fixture", "");
    addParameter(m_impl->fixtureParam);

    m_impl->enabledParam = new keira::BooleanParameter(EnabledParam, "Identify", false);
    addParameter(m_impl->enabledParam);

    m_impl->dmxOutParam = new DMXMatrixParameter(OutputDMX, "DMX Output", DMXMatrix(), keira::AllowMultipleOutput);
    addParameter(m_impl->dmxOutParam);
}

void IdentifyFixtureNode::setIdentifiedFixture(const QByteArray &t_fixtureId)
{
    m_impl->fixtureParam->setValue(t_fixtureId);
}

void IdentifyFixtureNode::setIdentifyEnabled(bool t_enabled)
{
    m_impl->enabledParam->setValue(t_enabled);
}

bool IdentifyFixtureNode::isIdentifyEnabled() const
{
    return m_impl->enabledParam->value().toBool();
}

void IdentifyFixtureNode::evaluate(keira::EvaluationContext *) const
{
    DMXMatrix matrix = m_impl->dmxInParam->value().value<DMXMatrix>();

    if(m_impl->enabledParam->value().toBool())
    {
        Fixture *fixture = FixtureCollection::fixtureById(m_impl->fixtureParam->value().toByteArray());
        if(fixture)
        {
            const auto dimmers = fixture->findCapability(Capability_Dimmer);
            if(!dimmers.isEmpty())
                static_cast<DimmerCapability*>(dimmers.first())->setPercent(1.0, matrix);

            if(auto *color = fixture->color())
                color->setColor(QColor(Qt::white), matrix);

            const auto shutters = fixture->findCapability(Capability_Strobe);
            for(auto *cap : shutters)
            {
                auto *shutter = static_cast<ShutterStrobeCapability*>(cap);
                if(shutter->shutterEffect() == ShutterStrobeCapability::Shutter_Open)
                {
                    matrix.setValue(shutter->channel()->universe() - 1,
                                     shutter->channel()->universalChannelNumber(),
                                     shutter->range().middle(), 1.0);
                    break;
                }
            }
        }
    }

    m_impl->dmxOutParam->setValue(matrix);
}

} // namespace photon
