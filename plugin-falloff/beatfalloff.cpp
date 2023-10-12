#include "beatfalloff.h"
#include "sequence/clip.h"
#include "sequence/sequence.h"
#include "sequence/beatlayer.h"

namespace photon {

FalloffEffectInformation BeatFalloff::info()
{
    FalloffEffectInformation toReturn([](){return new BeatFalloff;});
    toReturn.name = "Beat";
    toReturn.effectId = "photon.falloff.beat";
    toReturn.categories.append("Modifier");

    return toReturn;
}

BeatFalloff::BeatFalloff()
{

}

void BeatFalloff::rebuildCache()
{

}

double BeatFalloff::falloff(Fixture *t_fixture) const
{

    const auto &layers = clip()->sequence()->beatLayers();
    if(layers.empty())
        return 0.0;

    QVector<double> m_offsets;
    QHash<Fixture*, double> m_map;

    m_offsets.clear();
    auto fixtures = clip()->maskedFixtures();

    for(auto fix : fixtures)
    {
        m_map.insert(fix, previousEffect()->falloff(fix));
    }

    std::sort(fixtures.begin(), fixtures.end(),[m_map](Fixture *a, Fixture *b){return m_map.value(a) < m_map.value(b);});
    m_map.clear();

    const auto &layer = layers.front();
    double start = clip()->startTime();

    auto fixIt = fixtures.cbegin();
    for(auto it = layer->beats().constBegin(); it != layer->beats().constEnd() && fixIt != fixtures.cend(); ++it)
    {
        if(*it < start)
        {
            continue;
        }

        if(t_fixture == *fixIt)
            return (*it) - start;
        ++fixIt;

        if(m_offsets.length() >= fixtures.length())
            return 0.0;
    }

    return 0.0;
}

void BeatFalloff::readFromJson(const QJsonObject &)
{

}

void BeatFalloff::writeToJson(QJsonObject &) const{

}

} // namespace photon
