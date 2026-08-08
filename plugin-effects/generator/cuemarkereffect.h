#ifndef PHOTON_CUEMARKEREFFECT_H
#define PHOTON_CUEMARKEREFFECT_H

#include <QEasingCurve>
#include "sequence/channeleffect.h"

class QComboBox;

namespace photon {

class CueMarkerEffect;

class CueMarkerEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    CueMarkerEffectEditor(CueMarkerEffect *);

private slots:
    void layerChanged(int);
    void onValueChanged(double);
    void offValueChanged(double);
    void holdDurationChanged(double);
    void falloffTypeChanged(int);
    void falloffDurationChanged(double);
    void refreshLayerCombo();
    void layerMarkersChanged();

private:
    // Repaint-on-external-change (see layerMarkersChanged()): CueMarkerEffect
    // itself isn't a QObject and reads the layer live rather than caching it, so
    // nothing tells the curve view a marker moved unless the editor - which does
    // have a Qt connection to the live CueLayer - does it.
    void connectToLayer(CueLayer *);

    CueMarkerEffect *m_effect;
    QComboBox *m_layerCombo;
    CueLayer *m_connectedLayer = nullptr;
};

// Reads markers from a chosen CueLayer: crossing a marker jumps the output to
// onValue, holds it there for holdDuration, then eases back to offValue over
// falloffDuration along falloffType. A pure generator (like BeatIntegerEffect) -
// it replaces the upstream chain's value rather than building on it.
//
// CueLayer has no persistent unique id (see cuelayer.h), so the selected layer
// is persisted/resolved by name (resolveLayer()) rather than by pointer - a
// renamed layer will need to be reselected.
class CueMarkerEffect : public ChannelEffect
{
public:
    CueMarkerEffect();

    void setLayerName(const QString &);
    QString layerName() const { return m_layerName; }
    void setOnValue(double);
    double onValue() const { return m_onValue; }
    void setOffValue(double);
    double offValue() const { return m_offValue; }
    void setHoldDuration(double);
    double holdDuration() const { return m_holdDuration; }
    void setFalloffType(QEasingCurve::Type);
    QEasingCurve::Type falloffType() const { return m_falloffType; }
    void setFalloffDuration(double);
    double falloffDuration() const { return m_falloffDuration; }

    // The CueLayer currently resolved from layerName() (by name lookup - see the
    // class comment), or nullptr. Public so the editor can connect to its
    // markersChanged/metadataChanged to know when to repaint the curve.
    CueLayer *resolveLayer() const;

    float *process(float *value, uint size, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();

private:
    double computeValue(double globalTime) const;

    QString m_layerName;
    double m_onValue = 1.0;
    double m_offValue = 0.0;
    double m_holdDuration = 0.1;
    QEasingCurve::Type m_falloffType = QEasingCurve::OutQuad;
    double m_falloffDuration = 0.5;
    QEasingCurve m_falloffCurve;
};

} // namespace photon

#endif // PHOTON_CUEMARKEREFFECT_H
