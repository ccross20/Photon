#ifndef PHOTON_BEATINTEGEREFFECT_H
#define PHOTON_BEATINTEGEREFFECT_H

#include <QRandomGenerator>
#include "sequence/channeleffect.h"

namespace photon {


class BeatIntegerEffect;

class BeatIntegerEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    BeatIntegerEffectEditor(BeatIntegerEffect *);

private slots:
    void minRangeChanged(int);
    void maxRangeChanged(int);
    void modeChanged(int);
    void incrementEveryChanged(int);
    void startBeatChanged(int);

private:
    BeatIntegerEffect *m_effect;
};



class BeatIntegerEffect : public ChannelEffect
{
public:

    enum BeatIntegerMode{
        ModeIncrement,
        ModeDecrement,
        ModeRandom
    };

    BeatIntegerEffect();

    void setMinRange(int);
    void setMaxRange(int);
    void setMode(BeatIntegerMode);
    void setIncrementEvery(int);
    void setStartBeat(int);

    int minRange() const;
    int maxRange() const;
    BeatIntegerMode mode() const;
    int incrementEvery() const;
    int startBeat() const;


    float * process(float *value, uint size, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;


    static EffectInformation info();

protected:
    void addedToChannel() override;

private:

    struct BeatCell
    {
        float time;
        int value;
    };

    void rebuildCells();

    int m_min = 0;
    int m_max = 1000;
    BeatIntegerMode m_mode = ModeRandom;
    // Group beats into chunks of this size - only the first beat of each chunk
    // triggers a value change, the rest hold. "Start On Beat" (1-based) offsets
    // which beat the grouping starts counting from; beats before it hold the
    // initial value (m_min for Increment/Random, m_max for Decrement).
    int m_incrementEvery = 1;
    int m_startBeat = 1;
    QList<BeatCell> m_cells;
};

} // namespace photon

#endif // PHOTON_BEATINTEGEREFFECT_H
