#include "beatfalloff.h"
#include "sequence/fixtureclip.h"
#include "sequence/sequence.h"
#include "audio/songdata.h"

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
    FixtureClip *clip = dynamic_cast<FixtureClip*>(parent());

    if(!clip)
        return 0.0;

    // Beats come from the sequence's analysed beat grid, not a BeatLayer: layers
    // are reserved for user-authored custom cues, while the beat grid is derived
    // data that's always present once a file has been loaded and analysed.
    SongData *songData = clip->sequence()->songData();
    if(!songData || songData->beats().isEmpty())
        return 0.0;

    QVector<double> m_offsets;
    QHash<Fixture*, double> m_map;

    m_offsets.clear();
    auto fixtures = clip->maskedFixtures();

    for(auto fix : fixtures)
    {
        m_map.insert(fix, previousEffect()->falloff(fix));
    }

    std::sort(fixtures.begin(), fixtures.end(),[m_map](Fixture *a, Fixture *b){return m_map.value(a) < m_map.value(b);});
    m_map.clear();

    const QVector<double> &beats = songData->beats().beats();
    double start = clip->startTime();

    auto fixIt = fixtures.cbegin();
    for(auto it = beats.constBegin(); it != beats.constEnd() && fixIt != fixtures.cend(); ++it)
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
