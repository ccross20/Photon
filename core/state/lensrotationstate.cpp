#include "lensrotationstate.h"
#include "fixture/capability/wheelslotrotationcapability.h"
#include "fixture/fixture.h"

namespace photon {

LensRotationState::LensRotationState() : StateCapability(CapabilityType::Capability_LensRotation)
{
    setName("Lens Rotation");

    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeString, "Name","Name of the capability",""));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeNumber, "Speed","How fast to rotate the lens plate",0));
    addAvailableChannel(ChannelInfo(ChannelInfo::ChannelTypeBool, "Use Angle","Use angle instead of speed",false));

}

void LensRotationState::evaluate(const StateEvaluationContext &t_context) const
{

    float rotationAmount = getChannelFloat(t_context,1);

    bool useAngle = getChannelBool(t_context,2);
    QString name = getChannelString(t_context,0);

    if(!name.isEmpty())
    {
        auto allCapabilities = t_context.fixture->findCapability(CapabilityType::Capability_LensRotation, name);

        for(auto capability : allCapabilities)
        {
            auto lensRotation = static_cast<WheelSlotRotationCapability*>(capability);

            if(lensRotation->supportsAngle() && useAngle)
            {
                lensRotation->setAngleDegrees(rotationAmount,t_context.dmxMatrix,t_context.strength);
                return;
            }
            else
            {
                if(rotationAmount > 0 && lensRotation->maxSpeed() > 0)
                {
                    lensRotation->setSpeed(rotationAmount,t_context.dmxMatrix,t_context.strength);
                    return;
                }
                else if(rotationAmount < 0 && lensRotation->maxSpeed() < 0)
                {
                    lensRotation->setSpeed(rotationAmount,t_context.dmxMatrix,t_context.strength);
                    return;
                }
            }
        }
    }

}

} // namespace photon
