#ifndef PHOTON_SONGFEATUREEFFECT_H
#define PHOTON_SONGFEATUREEFFECT_H

#include "sequence/channeleffect.h"

namespace photon {

class SongFeatureEffect;

class SongFeatureEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    SongFeatureEffectEditor(SongFeatureEffect *);

private slots:
    void featureChanged(int);
    void scaleChanged(double);
    void offsetChanged(double);

private:
    SongFeatureEffect *m_effect;
};


// Reads a value straight out of the sequence's SongData (beat-synced level/
// frequency envelopes from LevelAnalysisProcess, or level/stem envelopes from a
// VirtualDJCaptureProcess live capture) at the channel's current song time,
// scales/offsets it, and adds it into the chain - the same additive pattern as the
// other generators (Sine, Noise, ...), just driven by analysed audio instead of a
// formula. Lets a dimmer/color channel track the track's bass hits, vocal presence,
// overall loudness, etc. Which features actually have data depends on how the
// sequence's SongData was produced - a local-file analysis populates Low/Mid/High,
// a VirtualDJ capture populates the stems instead.
class SongFeatureEffect : public ChannelEffect
{
public:
    enum Feature
    {
        FeatureLevel,
        FeatureLow,
        FeatureMid,
        FeatureHigh,
        FeatureVocal,
        FeatureInstru,
        FeatureBass,
        FeatureKick,
        FeatureHiHat
    };

    SongFeatureEffect();

    void setFeature(Feature);
    Feature feature() const { return m_feature; }
    void setScale(double);
    double scale() const { return m_scale; }
    void setOffset(double);
    double offset() const { return m_offset; }

    float *process(float *value, uint size, double time) const override;
    bool providesIsolatedContribution() const override { return true; }
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();

private:
    QByteArray featureId() const;

    Feature m_feature = FeatureLevel;
    double m_scale = 1.0;
    double m_offset = 0.0;
};

} // namespace photon

#endif // PHOTON_SONGFEATUREEFFECT_H
