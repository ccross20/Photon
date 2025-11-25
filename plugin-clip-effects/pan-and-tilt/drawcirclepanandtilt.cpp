#include "drawcirclepanandtilt.h"
#include "channel/parameter/numberchannelparameter.h"
#include "fixture/fixture.h"
#include "fixture/capability/anglecapability.h"
#include "sequence/fixtureclip.h"

namespace photon {

DrawCirclePanAndTilt::DrawCirclePanAndTilt() {}

void DrawCirclePanAndTilt::init()
{
    BaseEffect::init();
    addChannelParameter(new NumberChannelParameter("radius",5));
    addChannelParameter(new NumberChannelParameter("speed",2));
    addChannelParameter(new NumberChannelParameter("tiltOffset"));
    addChannelParameter(new NumberChannelParameter("panOffset"));
}

void DrawCirclePanAndTilt::evaluate(FixtureEffectEvaluationContext &t_context)
{
    float panOffset = t_context.channelValues["panOffset"].toDouble();
    float tiltOffset = t_context.channelValues["tiltOffset"].toDouble();
    float radius = t_context.channelValues["radius"].toDouble();
    float speed = t_context.channelValues["speed"].toDouble();


    double initialRelativeTime = t_context.globalTime - effectParent()->startTime();

    double panDeg = (std::sin(initialRelativeTime*(1.0/speed)) * radius)+panOffset;
    double tiltDeg = -(std::cos(initialRelativeTime*(1.0/speed)) * radius)+tiltOffset;

    //qDebug() << panDeg << tiltDeg;

    auto fixture = t_context.fixture;
    auto pans = fixture->findCapability(Capability_Pan);
    if(!pans.isEmpty())
        static_cast<AngleCapability*>(pans.front())->setAngleDegreesCentered(panDeg, t_context.dmxMatrix, effectParent()->strengthAtTime(initialRelativeTime));

    auto tilts = fixture->findCapability(Capability_Tilt);
    if(!tilts.isEmpty())
        static_cast<AngleCapability*>(tilts.front())->setAngleDegreesCentered(tiltDeg, t_context.dmxMatrix, effectParent()->strengthAtTime(initialRelativeTime));

}

ClipEffectInformation DrawCirclePanAndTilt::info()
{
    ClipEffectInformation toReturn([](){return new DrawCirclePanAndTilt;});
    toReturn.name = "Draw Circle";
    toReturn.id = "photon.clip.circle-pan-tilt";
    toReturn.categories.append("Position");

    return toReturn;
}


} // namespace photon
