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

    int minRange() const;
    int maxRange() const;
    BeatIntegerMode mode() const;


    double process(double value, double time) const override;
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
    QList<BeatCell> m_cells;
};

} // namespace photon

#endif // PHOTON_BEATINTEGEREFFECT_H
