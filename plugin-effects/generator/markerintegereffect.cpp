#include <algorithm>
#include <QComboBox>
#include <QJsonObject>
#include <QRandomGenerator>
#include "markerintegereffect.h"
#include "sequence/channel.h"
#include "sequence/sequence.h"
#include "sequence/cuelayer.h"
#include "propertywidgets.h"

namespace photon {

EffectInformation MarkerIntegerEffect::info()
{
    EffectInformation toReturn([](){return new MarkerIntegerEffect;});
    toReturn.name = "Marker Integer";
    toReturn.effectId = "photon.effect.marker-integer";
    toReturn.categories.append("Generator");
    return toReturn;
}

MarkerIntegerEffect::MarkerIntegerEffect() : ChannelEffect()
{
}

void MarkerIntegerEffect::setLayerId(const QByteArray &t_value)
{
    m_layerId = t_value;
    updated();
}

void MarkerIntegerEffect::setMinRange(int t_value)
{
    m_min = t_value;
    updated();
}

void MarkerIntegerEffect::setMaxRange(int t_value)
{
    m_max = t_value;
    updated();
}

void MarkerIntegerEffect::setMode(MarkerIntegerMode t_value)
{
    m_mode = t_value;
    updated();
}

void MarkerIntegerEffect::setIncrementEvery(int t_value)
{
    m_incrementEvery = std::max(t_value, 1);
    updated();
}

void MarkerIntegerEffect::setStartMarker(int t_value)
{
    m_startMarker = std::max(t_value, 1);
    updated();
}

CueLayer *MarkerIntegerEffect::resolveLayer() const
{
    Channel *ch = channel();
    Sequence *seq = ch ? ch->sequence() : nullptr;
    if(!seq)
        return nullptr;

    if(!m_layerId.isEmpty())
    {
        for(auto *layer : seq->cueLayers())
        {
            if(layer->uniqueId() == m_layerId)
                return layer;
        }
        return nullptr;
    }

    return seq->cueLayers().isEmpty() ? nullptr : seq->cueLayers().first();
}

int MarkerIntegerEffect::valueAt(double t_globalTime) const
{
    const int initial = m_mode == ModeDecrement ? m_max : m_min;

    CueLayer *layer = resolveLayer();
    if(!layer)
        return initial;

    // Sorted ascending (CueLayer keeps them so). The most recent marker at or
    // before this time sets the value.
    const QList<float> &markers = layer->markers();
    const auto it = std::upper_bound(markers.cbegin(), markers.cend(), static_cast<float>(t_globalTime));
    const int index = int(it - markers.cbegin()) - 1;   // -1 = before the first marker

    const int startIndex = m_startMarker - 1;
    if(index < startIndex)
        return initial;

    // Chunk boundaries crossed: 0 for the chunk the start marker opens.
    const int steps = (index - startIndex) / std::max(m_incrementEvery, 1);
    const int rangeSize = std::max(m_max - m_min + 1, 1);

    switch(m_mode)
    {
    case ModeIncrement:
        return m_min + steps % rangeSize;
    case ModeDecrement:
        return m_max - steps % rangeSize;
    case ModeRandom:
    default:
    {
        // One draw per chunk, seeded by the chunk number, so it's repeatable
        // and stays the same however the playhead jumps around. bounded()
        // asserts on an empty range, which min/max scrubbed past each other
        // would give.
        if(m_max <= m_min)
            return m_min;
        QRandomGenerator generator(quint32(123 + steps));
        return generator.bounded(m_min, m_max + 1);
    }
    }
}

float *MarkerIntegerEffect::process(float *value, uint size, double t_time) const
{
    // A pure generator: replaces the upstream chain's value, like Beat Integer.
    const float result = static_cast<float>(valueAt(t_time + channel()->startTime()));
    for(uint i = 0; i < size; ++i)
        value[i] = result;
    return value;
}

ChannelEffectEditor *MarkerIntegerEffect::createEditor()
{
    // The plain curve view, repainted when markers move: this effect isn't a
    // QObject, so nothing else tells the editor the curve changed. Every
    // layer is watched, so switching layers while it's open still repaints.
    auto *editor = new ChannelEffectEditor(this);
    if(Channel *ch = channel())
    {
        if(Sequence *seq = ch->sequence())
        {
            auto watch = [this, editor](CueLayer *t_layer){
                QObject::connect(t_layer, &CueLayer::markersChanged, editor, [this](CueLayer *){ updated(); });
            };
            for(auto *layer : seq->cueLayers())
                watch(layer);
            QObject::connect(seq, &Sequence::cueLayerAdded, editor, watch);
            QObject::connect(seq, &Sequence::cueLayerRemoved, editor, [this](CueLayer *){ updated(); });
        }
    }
    return editor;
}

QWidget *MarkerIntegerEffect::createPropertyEditor()
{
    auto *form = new PropertyForm;

    // The layer list changes as layers are added/removed, so it's built and
    // kept live here rather than through PropertyWidgets::createOptions (which
    // bakes its items in once) - same as Cue Marker's.
    auto *layerCombo = new QComboBox;
    auto refreshLayerCombo = [this, layerCombo]() {
        CueLayer *current = resolveLayer();
        layerCombo->blockSignals(true);
        layerCombo->clear();
        if(Channel *ch = channel())
        {
            if(Sequence *seq = ch->sequence())
            {
                for(auto *layer : seq->cueLayers())
                    layerCombo->addItem(layer->name(), QString::fromUtf8(layer->uniqueId()));
            }
        }
        layerCombo->setCurrentIndex(current ? layerCombo->findData(QString::fromUtf8(current->uniqueId())) : -1);
        layerCombo->blockSignals(false);
    };
    refreshLayerCombo();
    QObject::connect(layerCombo, &QComboBox::currentIndexChanged, layerCombo, [this, layerCombo](int index){
        if(index < 0)
            return;
        setLayerId(layerCombo->itemData(index).toString().toUtf8());
    });
    if(Channel *ch = channel())
    {
        if(Sequence *seq = ch->sequence())
        {
            QObject::connect(seq, &Sequence::cueLayerAdded, layerCombo, [refreshLayerCombo](CueLayer *){ refreshLayerCombo(); });
            QObject::connect(seq, &Sequence::cueLayerRemoved, layerCombo, [refreshLayerCombo](CueLayer *){ refreshLayerCombo(); });
        }
    }
    form->addRow("Layer", layerCombo);

    form->addRow("Minimum", PropertyWidgets::createInteger(m_min,
        {{PropertyWidgets::MetaMinimum, -100000.0}, {PropertyWidgets::MetaMaximum, 100000.0}, {PropertyWidgets::MetaSoftMinimum, 0.0}, {PropertyWidgets::MetaSoftMaximum, 16.0}},
        [this](int v){ setMinRange(v); }));
    form->addRow("Maximum", PropertyWidgets::createInteger(m_max,
        {{PropertyWidgets::MetaMinimum, -100000.0}, {PropertyWidgets::MetaMaximum, 100000.0}, {PropertyWidgets::MetaSoftMinimum, 0.0}, {PropertyWidgets::MetaSoftMaximum, 16.0}},
        [this](int v){ setMaxRange(v); }));
    form->addRow("Mode", PropertyWidgets::createOptions({"Increment", "Decrement", "Random"}, m_mode, {},
        [this](int v){ setMode(static_cast<MarkerIntegerMode>(v)); }));
    form->addRow("Increment Every (n) Markers", PropertyWidgets::createInteger(m_incrementEvery,
        {{PropertyWidgets::MetaMinimum, 1.0}, {PropertyWidgets::MetaMaximum, 1000.0}, {PropertyWidgets::MetaSoftMinimum, 1.0}, {PropertyWidgets::MetaSoftMaximum, 16.0}},
        [this](int v){ setIncrementEvery(v); }));
    form->addRow("Start On Marker", PropertyWidgets::createInteger(m_startMarker,
        {{PropertyWidgets::MetaMinimum, 1.0}, {PropertyWidgets::MetaMaximum, 100000.0}, {PropertyWidgets::MetaSoftMinimum, 1.0}, {PropertyWidgets::MetaSoftMaximum, 64.0}},
        [this](int v){ setStartMarker(v); }));

    return form;
}

void MarkerIntegerEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    m_layerId = t_json.value("layerId").toString().toUtf8();
    m_min = t_json.value("min").toInt(m_min);
    m_max = t_json.value("max").toInt(m_max);
    m_mode = static_cast<MarkerIntegerMode>(std::clamp(t_json.value("mode").toInt(m_mode), 0, int(ModeRandom)));
    m_incrementEvery = std::max(t_json.value("incrementEvery").toInt(m_incrementEvery), 1);
    m_startMarker = std::max(t_json.value("startMarker").toInt(m_startMarker), 1);
}

void MarkerIntegerEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    // The layer actually being read, so a default pick stays put if layers
    // are added or reordered later.
    QByteArray layerId = m_layerId;
    if(layerId.isEmpty())
    {
        if(CueLayer *layer = resolveLayer())
            layerId = layer->uniqueId();
    }
    t_json.insert("layerId", QString::fromUtf8(layerId));
    t_json.insert("min", m_min);
    t_json.insert("max", m_max);
    t_json.insert("mode", m_mode);
    t_json.insert("incrementEvery", m_incrementEvery);
    t_json.insert("startMarker", m_startMarker);
}

} // namespace photon
