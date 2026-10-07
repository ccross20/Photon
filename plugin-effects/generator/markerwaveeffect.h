#ifndef PHOTON_MARKERWAVEEFFECT_H
#define PHOTON_MARKERWAVEEFFECT_H

#include <QEasingCurve>
#include "sequence/channeleffect.h"

namespace photon {

class CueLayer;

// A wave timed by a cue layer's markers: each marker is a crest (Max) or a
// trough (Min), alternating, with the value easing smoothly from one to the
// next - so the wave's speed follows the markers' spacing. Start Direction
// sets whether it rises (Up: first marker a trough) or falls (Down: first a
// crest) toward the second marker. Shape is the curve between markers (In
// Out Sine for a true smooth wave, Linear for a triangle). Every (n) Markers
// uses only every nth marker; Start On Marker (1-based) is where it begins.
// Before the first marker it holds the first extreme, after the last, the
// last.
//
// Markers are read live (no cached table), and the layer is resolved and
// pinned the same way as Marker Integer's.
class MarkerWaveEffect : public ChannelEffect
{
public:
    enum Direction {
        DirectionUp,
        DirectionDown
    };

    MarkerWaveEffect();

    void setLayerId(const QByteArray &);
    QByteArray layerId() const { return m_layerId; }
    void setMinimum(double);
    double minimum() const { return m_min; }
    void setMaximum(double);
    double maximum() const { return m_max; }
    void setDirection(Direction);
    Direction direction() const { return m_direction; }
    void setShape(QEasingCurve::Type);
    QEasingCurve::Type shape() const { return m_shape; }
    void setEvery(int);
    int every() const { return m_every; }
    void setStartMarker(int);
    int startMarker() const { return m_startMarker; }

    // The layer read from: layerId() if it resolves, else the sequence's
    // first cue layer while none has been chosen, else null.
    CueLayer *resolveLayer() const;

    // The value at a song (global) time.
    double valueAt(double globalTime) const;

    float *process(float *value, uint size, double time) const override;
    ChannelEffectEditor *createEditor() override;
    QWidget *createPropertyEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();

private:
    // Min or Max at the wave's j-th marker.
    double extremeAt(int j) const;

    QByteArray m_layerId;
    double m_min = 0.0;
    double m_max = 1.0;
    Direction m_direction = DirectionUp;
    QEasingCurve::Type m_shape = QEasingCurve::InOutSine;
    QEasingCurve m_curve{QEasingCurve::InOutSine};
    int m_every = 1;
    int m_startMarker = 1;
};

} // namespace photon

#endif // PHOTON_MARKERWAVEEFFECT_H
