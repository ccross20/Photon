#ifndef PHOTON_MARKERINTEGEREFFECT_H
#define PHOTON_MARKERINTEGEREFFECT_H

#include "sequence/channeleffect.h"

namespace photon {

class CueLayer;

// Beat Integer's counterpart for a cue layer: steps an integer on each marker
// of the chosen layer instead of on each analysed beat. The value changes as
// the playhead crosses a marker and holds until the next one; before the
// first marker (and before "Start On Marker") it holds the initial value -
// Minimum for Increment/Random, Maximum for Decrement.
//
// Markers are read live from the layer on every process() call (no cached
// table to go stale), so moving, adding or removing markers takes effect
// immediately. The layer is persisted by CueLayer::uniqueId(); until one is
// chosen it reads the sequence's first cue layer, as Cue Marker does.
class MarkerIntegerEffect : public ChannelEffect
{
public:
    enum MarkerIntegerMode {
        ModeIncrement,
        ModeDecrement,
        ModeRandom
    };

    MarkerIntegerEffect();

    void setLayerId(const QByteArray &);
    QByteArray layerId() const { return m_layerId; }
    void setMinRange(int);
    int minRange() const { return m_min; }
    void setMaxRange(int);
    int maxRange() const { return m_max; }
    void setMode(MarkerIntegerMode);
    MarkerIntegerMode mode() const { return m_mode; }
    void setIncrementEvery(int);
    int incrementEvery() const { return m_incrementEvery; }
    void setIncrementAmount(int);
    int incrementAmount() const { return m_incrementAmount; }
    void setStartMarker(int);
    int startMarker() const { return m_startMarker; }

    // The layer read from: layerId() if it resolves, else the sequence's
    // first cue layer while none has been chosen, else null.
    CueLayer *resolveLayer() const;

    // The value at a song (global) time.
    int valueAt(double globalTime) const;

    float *process(float *value, uint size, double time) const override;
    ChannelEffectEditor *createEditor() override;
    QWidget *createPropertyEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();

private:
    QByteArray m_layerId;
    int m_min = 0;
    int m_max = 16;
    MarkerIntegerMode m_mode = ModeIncrement;
    // Markers are grouped into chunks of this size; only the first marker of
    // each chunk changes the value. "Start On Marker" (1-based) is where the
    // grouping starts counting.
    int m_incrementEvery = 1;
    // How far each step moves the value (Increment/Decrement), wrapping
    // around the Minimum..Maximum range.
    int m_incrementAmount = 1;
    int m_startMarker = 1;
};

} // namespace photon

#endif // PHOTON_MARKERINTEGEREFFECT_H
