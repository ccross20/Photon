#include <algorithm>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include "view/numberscrubfield.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "beatintegereffect.h"
#include "sequence/channel.h"
#include "sequence/sequence.h"
#include "audio/songdata.h"

namespace photon {

BeatIntegerEffectEditor::BeatIntegerEffectEditor(BeatIntegerEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    auto *minSpin = new keira::NumberScrubField;
    minSpin->setIsInteger(true);
    minSpin->setMinimum(-100000);
    minSpin->setMaximum(100000);
    minSpin->setValue(m_effect->minRange());
    connect(minSpin, &keira::NumberScrubField::valueChanged, this, [this](double v){ minRangeChanged(int(v)); });

    auto *maxSpin = new keira::NumberScrubField;
    maxSpin->setIsInteger(true);
    maxSpin->setMinimum(-100000);
    maxSpin->setMaximum(100000);
    maxSpin->setValue(m_effect->maxRange());
    connect(maxSpin, &keira::NumberScrubField::valueChanged, this, [this](double v){ maxRangeChanged(int(v)); });

    QComboBox *modeCombo = new QComboBox;
    modeCombo->addItems(QStringList{"Increment","Decrement","Random"});
    modeCombo->setCurrentIndex(static_cast<int>(m_effect->mode()));
    connect(modeCombo, &QComboBox::currentIndexChanged, this, &BeatIntegerEffectEditor::modeChanged);

    auto *incrementEverySpin = new keira::NumberScrubField;
    incrementEverySpin->setIsInteger(true);
    incrementEverySpin->setMinimum(1);
    incrementEverySpin->setMaximum(1000);
    incrementEverySpin->setValue(m_effect->incrementEvery());
    connect(incrementEverySpin, &keira::NumberScrubField::valueChanged, this, [this](double v){ incrementEveryChanged(int(v)); });

    auto *startBeatSpin = new keira::NumberScrubField;
    startBeatSpin->setIsInteger(true);
    startBeatSpin->setMinimum(1);
    startBeatSpin->setMaximum(100000);
    startBeatSpin->setValue(m_effect->startBeat());
    connect(startBeatSpin, &keira::NumberScrubField::valueChanged, this, [this](double v){ startBeatChanged(int(v)); });

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(minSpin, "Minimum");
    paramWidget->addWidget(maxSpin, "Maximum");
    paramWidget->addWidget(modeCombo, "Mode");
    paramWidget->addWidget(incrementEverySpin, "Increment Every (n) Beats");
    paramWidget->addWidget(startBeatSpin, "Start On Beat");


    addWidget(paramWidget, "Beat Integer");
}

void BeatIntegerEffectEditor::minRangeChanged(int t_value)
{
    m_effect->setMinRange(t_value);
}
void BeatIntegerEffectEditor::maxRangeChanged(int t_value)
{
    m_effect->setMaxRange(t_value);
}
void BeatIntegerEffectEditor::modeChanged(int t_value)
{
    m_effect->setMode(static_cast<BeatIntegerEffect::BeatIntegerMode>(t_value));
}

void BeatIntegerEffectEditor::incrementEveryChanged(int t_value)
{
    m_effect->setIncrementEvery(t_value);
}

void BeatIntegerEffectEditor::startBeatChanged(int t_value)
{
    m_effect->setStartBeat(t_value);
}

BeatIntegerEffect::BeatIntegerEffect()
{

}

void BeatIntegerEffect::setMinRange(int t_value)
{
    m_min = t_value;
    rebuildCells();
    updated();
}

void BeatIntegerEffect::setMaxRange(int t_value)
{
    m_max = t_value;
    rebuildCells();
    updated();
}

void BeatIntegerEffect::setMode(BeatIntegerMode t_value)
{
    m_mode = t_value;
    rebuildCells();
    updated();
}

void BeatIntegerEffect::setIncrementEvery(int t_value)
{
    m_incrementEvery = std::max(t_value, 1);
    rebuildCells();
    updated();
}

void BeatIntegerEffect::setStartBeat(int t_value)
{
    m_startBeat = std::max(t_value, 1);
    rebuildCells();
    updated();
}

int BeatIntegerEffect::minRange() const
{
    return m_min;
}

int BeatIntegerEffect::maxRange() const
{
    return m_max;
}

BeatIntegerEffect::BeatIntegerMode BeatIntegerEffect::mode() const
{
    return m_mode;
}

int BeatIntegerEffect::incrementEvery() const
{
    return m_incrementEvery;
}

int BeatIntegerEffect::startBeat() const
{
    return m_startBeat;
}

void BeatIntegerEffect::addedToChannel()
{
    ChannelEffect::addedToChannel();

    rebuildCells();
}

float * BeatIntegerEffect::process(float *value, uint size, double t_time) const
{
    for(int i = 0; i < size; ++i)
    {
        if(m_cells.isEmpty())
        {
            value[i] = m_min;
            continue;
        }
        float globalTime = t_time + channel()->startTime();

        BeatCell t;
        t.time  = globalTime;

        auto it = std::lower_bound(m_cells.cbegin(), m_cells.cend(), t,[](const BeatCell &a, const BeatCell &b){return a.time < b.time;});
        // Any query at or after the last beat lands on cend() - which is basically
        // every query past the final beat until the song ends. Hold the last cell's
        // value instead of dereferencing past the end (undefined behavior - reads
        // whatever garbage sits past the QList's buffer as the value).
        if(it == m_cells.cend())
            --it;

        value[i] = (*it).value;
    }

    return value;

}

void BeatIntegerEffect::rebuildCells()
{
    m_cells.clear();

    auto sequence = channel()->sequence();
    if(!sequence)
        return;

    // Beats come from the sequence's analysed beat grid, not a BeatLayer: layers
    // are reserved for user-authored custom cues, while the beat grid is derived
    // data that's always present once a file has been loaded and analysed.
    SongData *songData = sequence->songData();
    if(!songData || songData->beats().isEmpty())
        return;

    const QVector<double> &beats = songData->beats().beats();

    // "Start On Beat" is 1-based (beat 1 = the very first beat) - beats before it
    // just hold the initial value. From there, beats are grouped into chunks of
    // "Increment Every" size; only the first beat of each chunk actually advances
    // the value; the rest hold whatever the last chunk set. `steps` below is the
    // number of chunk-boundaries crossed by beat `i` - 0 for every beat in the
    // first (initial-value) chunk, 1 for the next chunk, and so on.
    const int startIndex = std::max(m_startBeat - 1, 0);
    const int incrementEvery = std::max(m_incrementEvery, 1);
    const auto stepsAt = [startIndex, incrementEvery](int i) {
        return i < startIndex ? 0 : (i - startIndex) / incrementEvery;
    };

    if(m_mode == ModeRandom)
    {
        QRandomGenerator generator(123);

        // bounded() requires a non-empty range (lowest < highest) and asserts
        // fatally otherwise. Min/Max are independent user-editable fields with no
        // cross-constraint in the UI, so a degenerate or inverted range (e.g. while
        // scrubbing one past the other) is reachable in normal use, not just a
        // theoretical edge case.
        const bool validRange = m_max > m_min;

        int val = m_min;
        int lastSteps = -1;
        for(int i = 0; i < beats.size(); ++i)
        {
            const int steps = stepsAt(i);
            // Draw exactly one random value per chunk (not per beat), so beats
            // within a chunk hold instead of each re-rolling - the generator has a
            // fixed seed, so this stays deterministic across rebuilds.
            if(i >= startIndex && steps != lastSteps)
            {
                val = validRange ? generator.bounded(m_min, m_max) : m_min;
                lastSteps = steps;
            }

            BeatCell cell;
            cell.time = static_cast<float>(beats[i]);
            cell.value = val;
            m_cells.append(cell);
        }
    }
    else if(m_mode == ModeIncrement)
    {
        const int rangeSize = std::max(m_max - m_min + 1, 1);
        for(int i = 0; i < beats.size(); ++i)
        {
            BeatCell cell;
            cell.time = static_cast<float>(beats[i]);
            cell.value = m_min + (stepsAt(i) % rangeSize);
            m_cells.append(cell);
        }
    }
    else
    {
        const int rangeSize = std::max(m_max - m_min + 1, 1);
        for(int i = 0; i < beats.size(); ++i)
        {
            BeatCell cell;
            cell.time = static_cast<float>(beats[i]);
            cell.value = m_max - (stepsAt(i) % rangeSize);
            m_cells.append(cell);
        }
    }
}

ChannelEffectEditor *BeatIntegerEffect::createEditor()
{
    return new BeatIntegerEffectEditor(this);
}

void BeatIntegerEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    m_min = t_json.value("min").toInt(m_min);
    m_max = t_json.value("max").toInt(m_max);
    m_mode = static_cast<BeatIntegerEffect::BeatIntegerMode>(t_json.value("mode").toInt(m_mode));
    m_incrementEvery = std::max(t_json.value("incrementEvery").toInt(m_incrementEvery), 1);
    m_startBeat = std::max(t_json.value("startBeat").toInt(m_startBeat), 1);
}

void BeatIntegerEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("min", m_min);
    t_json.insert("max", m_max);
    t_json.insert("mode", m_mode);
    t_json.insert("incrementEvery", m_incrementEvery);
    t_json.insert("startBeat", m_startBeat);
}

EffectInformation BeatIntegerEffect::info()
{
    EffectInformation toReturn([](){return new BeatIntegerEffect;});
    toReturn.name = "Beat Integer";
    toReturn.effectId = "photon.effect.beat-integer";
    toReturn.categories.append("Generator");

    return toReturn;
}

} // namespace photon
