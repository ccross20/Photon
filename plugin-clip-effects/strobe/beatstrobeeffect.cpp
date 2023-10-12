
#include <QRandomGenerator>
#include "beatstrobeeffect.h"
#include "channel/parameter/numberchannelparameter.h"
#include "channel/parameter/point2channelparameter.h"
#include "channel/parameter/colorchannelparameter.h"
#include "channel/parameter/boolchannelparameter.h"
#include "sequence/sequence.h"
#include "sequence/beatlayer.h"
#include "sequence/clip.h"
#include "fixture/fixture.h"
#include "fixture/capability/anglecapability.h"
#include "fixture/capability/dimmercapability.h"

namespace photon {

class BeatStrobeEffect::Impl{
public:

    struct BeatFixture{
        Fixture *fixture;
        float tilt = 0;
        float pan = 0;
        float dimmer = 0;
    };

    struct BeatCell
    {
        float time;
        QList<BeatFixture> fixtures;
        int nextGroup;
    };

    void setGroupCount(int);
    int groupCount = -1;
    double minTilt;
    double maxTilt;
    double minPan;
    double maxPan;

    QList<BeatCell> beatCells;
    BeatStrobeEffect *facade;
};

void BeatStrobeEffect::Impl::setGroupCount(int t_value)
{
    if(groupCount == t_value)
        return;

    groupCount = t_value;
    beatCells.clear();

    auto allFixtures = facade->clip()->maskedFixtures();
    auto beatLayers = facade->clip()->sequence()->beatLayers();
    if(beatLayers.isEmpty())
        return;
    auto beats = beatLayers.front()->beats();

    QList<Fixture*> lastFixtures;
    double probability = 1.0/ (static_cast<double>(groupCount) - 1.0);
    int counter = 0;

    BeatCell lastCell;
    for(auto fix : allFixtures)
    {

        BeatFixture beatFix;
        beatFix.fixture = fix;

        lastCell.fixtures << beatFix;
    }


    for(auto beat : beats)
    {
        BeatCell newCell = lastCell;
        newCell.time = beat;

        QRandomGenerator generator(counter++);

        QList<Fixture*> selectedFixtures;
        QList<Fixture*> possibleFixtures;


        if(groupCount == 1)
        {
            selectedFixtures = allFixtures;

            for(auto &beatFix : newCell.fixtures)
            {
                beatFix.pan = ((maxPan - minPan) * generator.generateDouble()) + minPan;
                beatFix.tilt = ((maxTilt - minTilt) * generator.generateDouble()) + minTilt;


                beatFix.dimmer = 1.0;

            }
        }
        else
        {
            for(auto fix : allFixtures)
            {
                if(!lastFixtures.contains(fix))
                    possibleFixtures << fix;
            }

            for(auto fix: possibleFixtures)
            {
                if(generator.generateDouble() >= probability)
                    selectedFixtures << fix;
            }

            for(auto &beatFix : newCell.fixtures)
            {
                if(selectedFixtures.contains(beatFix.fixture))
                {
                    beatFix.dimmer = 1.0;
                }
                else
                {
                    if(beatFix.dimmer > 0)
                    {
                        beatFix.pan = ((maxPan - minPan) * generator.generateDouble()) + minPan;
                        beatFix.tilt = ((maxTilt - minTilt) * generator.generateDouble()) + minTilt;
                    }

                    beatFix.dimmer = 0.0;
                }
            }
        }




        beatCells.append(newCell);

        lastCell = newCell;
        lastFixtures = selectedFixtures;
    }

    qDebug() << beatCells.count();
}

BeatStrobeEffect::BeatStrobeEffect():FixtureClipEffect(),m_impl(new Impl)
{
    m_impl->facade = this;
}

BeatStrobeEffect::~BeatStrobeEffect()
{
    delete m_impl;
}

void BeatStrobeEffect::init()
{
    addChannelParameter(new NumberChannelParameter("groups",3));
    addChannelParameter(new NumberChannelParameter("maxTilt",.2));
    addChannelParameter(new NumberChannelParameter("minTilt",-.2));
    addChannelParameter(new NumberChannelParameter("maxPan",.2));
    addChannelParameter(new NumberChannelParameter("minPan",-.2));
    addChannelParameter(new NumberChannelParameter("pulseDuration",.1));
}

void BeatStrobeEffect::evaluate(FixtureClipEffectEvaluationContext &t_context) const
{
    m_impl->minPan = t_context.channelValues["minPan"].toDouble();
    m_impl->maxPan = t_context.channelValues["maxPan"].toDouble();
    m_impl->minTilt = t_context.channelValues["minTilt"].toDouble();
    m_impl->maxTilt = t_context.channelValues["maxTilt"].toDouble();
    m_impl->setGroupCount(t_context.channelValues["groups"].toInt());

    double pulseDuration = t_context.channelValues["pulseDuration"].toDouble();

    float globalTime = t_context.globalTime;
    double initialRelativeTime = t_context.globalTime - clip()->startTime();

    Impl::BeatCell t;
    t.time  = globalTime;

    auto it = std::lower_bound(m_impl->beatCells.cbegin(), m_impl->beatCells.cend(), t,[](const Impl::BeatCell &a, const Impl::BeatCell &b){return a.time < b.time;});

    if(it != m_impl->beatCells.cbegin())
        it--;

    Impl::BeatCell cell = *it;
    Impl::BeatCell nextCell = cell;

    if(it != m_impl->beatCells.cend())
        nextCell = *(it+1);

    double distanceFromBeat = globalTime - cell.time;

    //qDebug() << distanceFromBeat << pulseDuration;

    if(distanceFromBeat > pulseDuration)
    {
        for(auto fix : nextCell.fixtures)
        {
            double relTime = initialRelativeTime - clip()->falloff(fix.fixture);


            auto dimmers = fix.fixture->findCapability(Capability_Dimmer);
            if(!dimmers.isEmpty())
                static_cast<DimmerCapability*>(dimmers.front())->setPercent(0, t_context.dmxMatrix, clip()->strengthAtTime(relTime));


            auto pans = fix.fixture->findCapability(Capability_Pan);
            if(!pans.isEmpty())
                static_cast<AngleCapability*>(pans.front())->setAnglePercentCentered(fix.pan, t_context.dmxMatrix, clip()->strengthAtTime(relTime));

            auto tilts = fix.fixture->findCapability(Capability_Tilt);
            if(!tilts.isEmpty())
                static_cast<AngleCapability*>(tilts.front())->setAnglePercentCentered(fix.tilt, t_context.dmxMatrix, clip()->strengthAtTime(relTime));

        }
    }
    else
    {
        for(auto fix : cell.fixtures)
        {
            double relTime = initialRelativeTime - clip()->falloff(fix.fixture);


            auto dimmers = fix.fixture->findCapability(Capability_Dimmer);
            if(!dimmers.isEmpty())
                static_cast<DimmerCapability*>(dimmers.front())->setPercent(fix.dimmer, t_context.dmxMatrix, clip()->strengthAtTime(relTime));


            auto pans = fix.fixture->findCapability(Capability_Pan);
            if(!pans.isEmpty())
                static_cast<AngleCapability*>(pans.front())->setAnglePercentCentered(fix.pan, t_context.dmxMatrix, clip()->strengthAtTime(relTime));

            auto tilts = fix.fixture->findCapability(Capability_Tilt);
            if(!tilts.isEmpty())
                static_cast<AngleCapability*>(tilts.front())->setAnglePercentCentered(fix.tilt, t_context.dmxMatrix, clip()->strengthAtTime(relTime));

        }
    }




}


ClipEffectInformation BeatStrobeEffect::info()
{
    ClipEffectInformation toReturn([](){return new BeatStrobeEffect;});
    toReturn.name = "Beat strobe";
    toReturn.id = "photon.clip.effect.beat-strobe";
    toReturn.categories.append("Strobe");

    return toReturn;
}

} // namespace photon
