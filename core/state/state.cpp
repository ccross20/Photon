#include "state.h"
#include "statecapability.h"
#include "shutterstate.h"
#include "tiltstate.h"
#include "tiltanglestate.h"
#include "tiltanglecenteredstate.h"
#include "dimmerstate.h"
#include "zoomstate.h"
#include "focusstate.h"
#include "panstate.h"
#include "colorstate.h"

namespace photon {

class State::Impl
{
public:
    StateCapability *addCapability(CapabilityType t_type);

    QVector<StateCapability*> capabilities;
    QByteArray uniqueId;
};

StateCapability *State::Impl::addCapability(CapabilityType t_type)
{
    StateCapability *toAdd = nullptr;
    switch (t_type) {
        case Capability_Strobe:
            toAdd = new ShutterState;
            break;
        case Capability_Tilt:
            toAdd = new TiltState;
            break;
        case Capability_TiltAngle:
            toAdd = new TiltAngleState;
            break;
        case Capability_TiltAngleCentered:
            toAdd = new TiltAngleCenteredState;
            break;
        case Capability_Dimmer:
            toAdd = new DimmerState;
            break;
        case Capability_Focus:
            toAdd = new FocusState;
            break;
        case Capability_Zoom:
            toAdd = new ZoomState;
            break;
        case Capability_Pan:
            toAdd = new PanState;
            break;
        case Capability_Color:
            toAdd = new ColorState;
            break;
        default:
            break;
    }

    if(toAdd)
        capabilities.append(toAdd);
    return toAdd;
}

State::State():m_impl(new Impl)
{
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
}

State::~State()
{
    delete m_impl;
}

QByteArray State::uniqueId() const
{
    return m_impl->uniqueId;
}

void State::addDefaultCapabilities()
{
    addCapability(Capability_Strobe);
    addCapability(Capability_Dimmer);
    addCapability(Capability_Tilt);
    addCapability(Capability_Pan);
    addCapability(Capability_Focus);
    addCapability(Capability_Zoom);
    addCapability(Capability_Color);
}

const QVector<StateCapability*> &State::capabilities() const
{
    return m_impl->capabilities;
}

StateCapability *State::addCapability(CapabilityType t_type)
{
    auto toAdd = m_impl->addCapability(t_type);
    emit capabilityAdded(toAdd);
    return toAdd;
}

void State::removeCapability(StateCapability *t_capability)
{
    m_impl->capabilities.removeOne(t_capability);
    emit capabilityRemoved(t_capability);
}

void State::initializeValues(StateEvaluationContext &t_context) const
{
    for(auto capability : m_impl->capabilities)
    {
        capability->initializeValues(t_context);
    }
}

void State::evaluate(const StateEvaluationContext &t_context) const
{
    for(auto capability : m_impl->capabilities)
    {
        if(capability->isEnabled())
            capability->evaluate(t_context);
    }
}

void State::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();

    if(t_json.contains("capabilities"))
    {
        auto capabilityArray = t_json.value("capabilities").toArray();
        for(auto capability : capabilityArray)
        {
            if(!capability.isObject())
                continue;
            QJsonObject capabilityObj = capability.toObject();

            if(!capabilityObj.contains("type"))
                continue;

            auto type = static_cast<CapabilityType>(capabilityObj.value("type").toInt());

            auto addedCapability = m_impl->addCapability(type);
            addedCapability->readFromJson(capabilityObj, t_context);
        }
    }

}

void State::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("uniqueId", QString(m_impl->uniqueId));

    QJsonArray capabilityArray;
    for(auto capability : m_impl->capabilities)
    {
        QJsonObject capabilityObj;
        capability->writeToJson(capabilityObj);
        capabilityArray.append(capabilityObj);
    }

    t_json.insert("capabilities", capabilityArray);
}

} // namespace photon
