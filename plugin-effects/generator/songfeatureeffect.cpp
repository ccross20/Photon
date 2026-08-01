#include <QComboBox>
#include "songfeatureeffect.h"
#include "sequence/channel.h"
#include "sequence/sequence.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "audio/songdata.h"
#include "view/numberscrubfield.h"

namespace photon {

SongFeatureEffectEditor::SongFeatureEffectEditor(SongFeatureEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    QComboBox *featureCombo = new QComboBox;
    featureCombo->addItems({"Level", "Low", "Mid", "High", "Vocal", "Instru", "Bass", "Kick", "HiHat"});
    featureCombo->setCurrentIndex(static_cast<int>(m_effect->feature()));
    connect(featureCombo, &QComboBox::currentIndexChanged, this, &SongFeatureEffectEditor::featureChanged);

    auto *scaleSpin = new keira::NumberScrubField;
    scaleSpin->setValue(m_effect->scale());
    connect(scaleSpin, &keira::NumberScrubField::valueChanged, this, &SongFeatureEffectEditor::scaleChanged);

    auto *offsetSpin = new keira::NumberScrubField;
    offsetSpin->setValue(m_effect->offset());
    connect(offsetSpin, &keira::NumberScrubField::valueChanged, this, &SongFeatureEffectEditor::offsetChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(featureCombo, "Feature");
    paramWidget->addWidget(scaleSpin, "Scale");
    paramWidget->addWidget(offsetSpin, "Offset");

    addWidget(paramWidget, "Song Feature");
}

void SongFeatureEffectEditor::featureChanged(int t_value)
{
    m_effect->setFeature(static_cast<SongFeatureEffect::Feature>(t_value));
}

void SongFeatureEffectEditor::scaleChanged(double t_value)
{
    m_effect->setScale(t_value);
}

void SongFeatureEffectEditor::offsetChanged(double t_value)
{
    m_effect->setOffset(t_value);
}


EffectInformation SongFeatureEffect::info()
{
    EffectInformation toReturn([](){return new SongFeatureEffect;});
    toReturn.name = "Song Feature";
    toReturn.effectId = "photon.effect.song-feature";
    toReturn.categories.append("Generator");

    return toReturn;
}

SongFeatureEffect::SongFeatureEffect() : ChannelEffect()
{
}

void SongFeatureEffect::setFeature(Feature t_value)
{
    m_feature = t_value;
    updated();
}

void SongFeatureEffect::setScale(double t_value)
{
    m_scale = t_value;
    updated();
}

void SongFeatureEffect::setOffset(double t_value)
{
    m_offset = t_value;
    updated();
}

QByteArray SongFeatureEffect::featureId() const
{
    switch(m_feature)
    {
    case FeatureLow:    return SongData::FeatureLow;
    case FeatureMid:    return SongData::FeatureMid;
    case FeatureHigh:   return SongData::FeatureHigh;
    case FeatureVocal:  return SongData::FeatureVocal;
    case FeatureInstru: return SongData::FeatureInstru;
    case FeatureBass:   return SongData::FeatureBass;
    case FeatureKick:   return SongData::FeatureKick;
    case FeatureHiHat:  return SongData::FeatureHiHat;
    default:            return SongData::FeatureLevel;
    }
}

float *SongFeatureEffect::process(float *value, uint size, double time) const
{
    if(previousEffect())
        value = previousEffect()->process(value, size, time);

    Channel *ch = channel();
    Sequence *seq = ch ? ch->sequence() : nullptr;
    SongData *songData = seq ? seq->songData() : nullptr;
    if(!songData)
        return value;

    // 'time' arrives relative to the channel's own start; SongData is indexed by
    // absolute song time, so translate before sampling (same convention as
    // BeatIntegerEffect's beat lookups).
    const double songTime = time + ch->startTime();
    const float sample = songData->featureValue(featureId(), songTime);
    const float delta = static_cast<float>(sample * m_scale + m_offset);

    for(int i = 0; i < size; ++i)
        value[i] += delta;

    return value;
}

ChannelEffectEditor *SongFeatureEffect::createEditor()
{
    return new SongFeatureEffectEditor(this);
}

void SongFeatureEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("feature"))
        m_feature = static_cast<Feature>(t_json.value("feature").toInt());
    if(t_json.contains("scale"))
        m_scale = t_json.value("scale").toDouble();
    if(t_json.contains("offset"))
        m_offset = t_json.value("offset").toDouble();
}

void SongFeatureEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("feature", static_cast<int>(m_feature));
    t_json.insert("scale", m_scale);
    t_json.insert("offset", m_offset);
}

} // namespace photon
