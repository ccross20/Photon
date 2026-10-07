#include <algorithm>
#include <QComboBox>
#include <QJsonObject>
#include "markerwaveeffect.h"
#include "sequence/channel.h"
#include "sequence/sequence.h"
#include "sequence/cuelayer.h"
#include "propertywidgets.h"
#include "util/utils.h"

namespace photon {

EffectInformation MarkerWaveEffect::info()
{
    EffectInformation toReturn([](){return new MarkerWaveEffect;});
    toReturn.name = "Marker Wave";
    toReturn.effectId = "photon.effect.marker-wave";
    toReturn.categories.append("Generator");
    return toReturn;
}

MarkerWaveEffect::MarkerWaveEffect() : ChannelEffect()
{
}

void MarkerWaveEffect::setLayerId(const QByteArray &t_value)
{
    m_layerId = t_value;
    updated();
}

void MarkerWaveEffect::setMinimum(double t_value)
{
    m_min = t_value;
    updated();
}

void MarkerWaveEffect::setMaximum(double t_value)
{
    m_max = t_value;
    updated();
}

void MarkerWaveEffect::setDirection(Direction t_value)
{
    m_direction = t_value;
    updated();
}

void MarkerWaveEffect::setShape(QEasingCurve::Type t_value)
{
    m_shape = t_value;
    m_curve.setType(t_value);
    updated();
}

void MarkerWaveEffect::setEvery(int t_value)
{
    m_every = std::max(t_value, 1);
    updated();
}

void MarkerWaveEffect::setStartMarker(int t_value)
{
    m_startMarker = std::max(t_value, 1);
    updated();
}

CueLayer *MarkerWaveEffect::resolveLayer() const
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

double MarkerWaveEffect::extremeAt(int t_j) const
{
    // Up: troughs on even markers (rising toward the next); Down: crests.
    const bool trough = (t_j % 2 == 0) == (m_direction == DirectionUp);
    return trough ? m_min : m_max;
}

double MarkerWaveEffect::valueAt(double t_globalTime) const
{
    CueLayer *layer = resolveLayer();
    if(!layer)
        return extremeAt(0);

    // Sorted ascending (CueLayer keeps them so).
    const QList<float> &markers = layer->markers();
    const int start = m_startMarker - 1;
    const int every = std::max(m_every, 1);
    if(start >= markers.size())
        return extremeAt(0);

    // The last marker at or before this time.
    const auto it = std::upper_bound(markers.cbegin(), markers.cend(), static_cast<float>(t_globalTime));
    const int index = int(it - markers.cbegin()) - 1;
    if(index < start)
        return extremeAt(0);   // before the wave's first marker

    // The wave marker this falls after (j), and the one it heads toward.
    const int j = (index - start) / every;
    const int from = start + j * every;
    const int to = from + every;
    if(to >= markers.size())
        return extremeAt(j);   // past the last: hold its extreme

    const double span = double(markers[to]) - double(markers[from]);
    if(span <= 0.0)
        return extremeAt(j + 1);
    const double progress = std::clamp((t_globalTime - markers[from]) / span, 0.0, 1.0);
    const double a = extremeAt(j);
    const double b = extremeAt(j + 1);
    return a + (b - a) * m_curve.valueForProgress(progress);
}

float *MarkerWaveEffect::process(float *value, uint size, double t_time) const
{
    // A pure generator: replaces the upstream chain's value.
    const float result = static_cast<float>(valueAt(t_time + channel()->startTime()));
    for(uint i = 0; i < size; ++i)
        value[i] = result;
    return value;
}

ChannelEffectEditor *MarkerWaveEffect::createEditor()
{
    // The plain curve view, repainted when markers move: this effect isn't a
    // QObject, so nothing else tells the editor the curve changed.
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

QWidget *MarkerWaveEffect::createPropertyEditor()
{
    auto *form = new PropertyForm;

    // Kept live as layers are added/removed - same as Marker Integer's.
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

    form->addRow("Min", PropertyWidgets::createNumber(m_min,
        {{PropertyWidgets::MetaSoftMinimum, 0.0}, {PropertyWidgets::MetaSoftMaximum, 1.0}},
        [this](double v){ setMinimum(v); }));
    form->addRow("Max", PropertyWidgets::createNumber(m_max,
        {{PropertyWidgets::MetaSoftMinimum, 0.0}, {PropertyWidgets::MetaSoftMaximum, 1.0}},
        [this](double v){ setMaximum(v); }));
    form->addRow("Start Direction", PropertyWidgets::createOptions({"Up", "Down"}, m_direction, {},
        [this](int v){ setDirection(static_cast<Direction>(v)); }));
    // Same list-index-equals-QEasingCurve::Type-value assumption as the
    // other effects' ease combos.
    form->addRow("Shape", PropertyWidgets::createOptions(easeStrings(), m_shape, {},
        [this](int v){ setShape(static_cast<QEasingCurve::Type>(v)); }));
    form->addRow("Every (n) Markers", PropertyWidgets::createInteger(m_every,
        {{PropertyWidgets::MetaMinimum, 1.0}, {PropertyWidgets::MetaMaximum, 1000.0}, {PropertyWidgets::MetaSoftMinimum, 1.0}, {PropertyWidgets::MetaSoftMaximum, 16.0}},
        [this](int v){ setEvery(v); }));
    form->addRow("Start On Marker", PropertyWidgets::createInteger(m_startMarker,
        {{PropertyWidgets::MetaMinimum, 1.0}, {PropertyWidgets::MetaMaximum, 100000.0}, {PropertyWidgets::MetaSoftMinimum, 1.0}, {PropertyWidgets::MetaSoftMaximum, 64.0}},
        [this](int v){ setStartMarker(v); }));

    return form;
}

void MarkerWaveEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    m_layerId = t_json.value("layerId").toString().toUtf8();
    m_min = t_json.value("min").toDouble(m_min);
    m_max = t_json.value("max").toDouble(m_max);
    m_direction = t_json.value("direction").toInt(m_direction) == DirectionDown ? DirectionDown : DirectionUp;
    m_shape = static_cast<QEasingCurve::Type>(t_json.value("shape").toInt(m_shape));
    m_curve.setType(m_shape);
    m_every = std::max(t_json.value("every").toInt(m_every), 1);
    m_startMarker = std::max(t_json.value("startMarker").toInt(m_startMarker), 1);
}

void MarkerWaveEffect::writeToJson(QJsonObject &t_json) const
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
    t_json.insert("direction", m_direction);
    t_json.insert("shape", int(m_shape));
    t_json.insert("every", m_every);
    t_json.insert("startMarker", m_startMarker);
}

} // namespace photon
