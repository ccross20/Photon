#include <algorithm>
#include <QComboBox>
#include "cuemarkereffect.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "numberscrubfield.h"
#include "sequence/channel.h"
#include "sequence/sequence.h"
#include "sequence/cuelayer.h"
#include "util/utils.h"
#include "propertywidgets.h"

namespace photon {

CueMarkerEffectEditor::CueMarkerEffectEditor(CueMarkerEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    m_layerCombo = new QComboBox;
    refreshLayerCombo();   // also resolves+connects the initial layer, see below
    connect(m_layerCombo, &QComboBox::currentIndexChanged, this, &CueMarkerEffectEditor::layerChanged);

    // Keep the layer list live while the editor is open, mirroring how
    // SequenceWaveformEditor tracks the same signals.
    if(Channel *ch = m_effect->channel())
    {
        if(Sequence *seq = ch->sequence())
        {
            connect(seq, &Sequence::cueLayerAdded, this, &CueMarkerEffectEditor::refreshLayerCombo);
            connect(seq, &Sequence::cueLayerRemoved, this, &CueMarkerEffectEditor::refreshLayerCombo);
        }
    }

    auto *onSpin = new photon::NumberScrubField;
    onSpin->setMinimum(-10000);
    onSpin->setMaximum(10000);
    onSpin->setValue(m_effect->onValue());
    connect(onSpin, &photon::NumberScrubField::valueChanged, this, &CueMarkerEffectEditor::onValueChanged);

    auto *offSpin = new photon::NumberScrubField;
    offSpin->setMinimum(-10000);
    offSpin->setMaximum(10000);
    offSpin->setValue(m_effect->offValue());
    connect(offSpin, &photon::NumberScrubField::valueChanged, this, &CueMarkerEffectEditor::offValueChanged);

    auto *holdSpin = new photon::NumberScrubField;
    holdSpin->setMinimum(0.0);
    holdSpin->setMaximum(9999);
    holdSpin->setValue(m_effect->holdDuration());
    connect(holdSpin, &photon::NumberScrubField::valueChanged, this, &CueMarkerEffectEditor::holdDurationChanged);

    // Same list-index-equals-QEasingCurve::Type-value assumption as
    // PeakHoldEffectEditor/EaseEffectEditor.
    QComboBox *falloffCombo = new QComboBox;
    falloffCombo->addItems(easeStrings());
    falloffCombo->setCurrentIndex(m_effect->falloffType());
    connect(falloffCombo, &QComboBox::currentIndexChanged, this, &CueMarkerEffectEditor::falloffTypeChanged);

    auto *falloffSpin = new photon::NumberScrubField;
    falloffSpin->setMinimum(0.0);
    falloffSpin->setMaximum(9999);
    falloffSpin->setValue(m_effect->falloffDuration());
    connect(falloffSpin, &photon::NumberScrubField::valueChanged, this, &CueMarkerEffectEditor::falloffDurationChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(m_layerCombo, "Layer");
    paramWidget->addWidget(onSpin, "On Value");
    paramWidget->addWidget(offSpin, "Off Value");
    paramWidget->addWidget(holdSpin, "Hold Duration");
    paramWidget->addWidget(falloffCombo, "Falloff Type");
    paramWidget->addWidget(falloffSpin, "Falloff Duration");

    addWidget(paramWidget, "Cue Marker");
}

void CueMarkerEffectEditor::refreshLayerCombo()
{
    CueLayer *current = m_effect->resolveLayer();

    m_layerCombo->blockSignals(true);
    m_layerCombo->clear();

    if(Channel *ch = m_effect->channel())
    {
        if(Sequence *seq = ch->sequence())
        {
            for(auto *layer : seq->cueLayers())
                m_layerCombo->addItem(layer->name(), QString::fromUtf8(layer->uniqueId()));
        }
    }

    if(current)
        m_layerCombo->setCurrentIndex(m_layerCombo->findData(QString::fromUtf8(current->uniqueId())));
    m_layerCombo->blockSignals(false);

    // The layer list can change (add/remove) without the selected one changing,
    // but resolveLayer() re-resolves regardless, and connectToLayer() is a
    // no-op if it's the same pointer - so this is always safe/cheap to redo.
    connectToLayer(current);
}

void CueMarkerEffectEditor::connectToLayer(CueLayer *t_layer)
{
    if(m_connectedLayer == t_layer)
        return;

    if(m_connectedLayer)
    {
        disconnect(m_connectedLayer, &CueLayer::markersChanged, this, &CueMarkerEffectEditor::layerMarkersChanged);
        disconnect(m_connectedLayer, &CueLayer::metadataChanged, this, &CueMarkerEffectEditor::layerMarkersChanged);
    }

    m_connectedLayer = t_layer;

    if(m_connectedLayer)
    {
        connect(m_connectedLayer, &CueLayer::markersChanged, this, &CueMarkerEffectEditor::layerMarkersChanged);
        connect(m_connectedLayer, &CueLayer::metadataChanged, this, &CueMarkerEffectEditor::layerMarkersChanged);
    }
}

void CueMarkerEffectEditor::layerMarkersChanged()
{
    // Nothing about the effect's own parameters changed - just repaint the curve
    // to reflect the layer's new marker positions.
    m_effect->updated();
}

void CueMarkerEffectEditor::layerChanged(int t_index)
{
    if(t_index < 0)
        return;
    m_effect->setLayerId(m_layerCombo->itemData(t_index).toString().toUtf8());
    connectToLayer(m_effect->resolveLayer());
}

void CueMarkerEffectEditor::onValueChanged(double t_value)
{
    m_effect->setOnValue(t_value);
}

void CueMarkerEffectEditor::offValueChanged(double t_value)
{
    m_effect->setOffValue(t_value);
}

void CueMarkerEffectEditor::holdDurationChanged(double t_value)
{
    m_effect->setHoldDuration(t_value);
}

void CueMarkerEffectEditor::falloffTypeChanged(int t_value)
{
    m_effect->setFalloffType(static_cast<QEasingCurve::Type>(t_value));
}

void CueMarkerEffectEditor::falloffDurationChanged(double t_value)
{
    m_effect->setFalloffDuration(t_value);
}

EffectInformation CueMarkerEffect::info()
{
    EffectInformation toReturn([](){return new CueMarkerEffect;});
    toReturn.name = "Cue Marker";
    toReturn.effectId = "photon.effect.cue-marker";
    toReturn.categories.append("Generator");

    return toReturn;
}

CueMarkerEffect::CueMarkerEffect() : ChannelEffect()
{
    m_falloffCurve.setType(m_falloffType);
}

void CueMarkerEffect::setLayerId(const QByteArray &t_value)
{
    m_layerId = t_value;
    updated();
}

void CueMarkerEffect::setOnValue(double t_value)
{
    m_onValue = t_value;
    updated();
}

void CueMarkerEffect::setOffValue(double t_value)
{
    m_offValue = t_value;
    updated();
}

void CueMarkerEffect::setHoldDuration(double t_value)
{
    m_holdDuration = std::max(t_value, 0.0);
    updated();
}

void CueMarkerEffect::setFalloffType(QEasingCurve::Type t_value)
{
    m_falloffType = t_value;
    m_falloffCurve.setType(t_value);
    updated();
}

void CueMarkerEffect::setFalloffDuration(double t_value)
{
    m_falloffDuration = std::max(t_value, 0.0);
    updated();
}

CueLayer *CueMarkerEffect::resolveLayer() const
{
    Channel *ch = channel();
    if(!ch)
        return nullptr;
    Sequence *seq = ch->sequence();
    if(!seq)
        return nullptr;

    if(!m_layerId.isEmpty())
    {
        for(auto *layer : seq->cueLayers())
            if(layer->uniqueId() == m_layerId)
                return layer;
    }

    // Back-compat for effects saved before CueLayer had a uniqueId.
    if(!m_legacyLayerName.isEmpty())
    {
        for(auto *layer : seq->cueLayers())
            if(layer->name() == m_legacyLayerName)
                return layer;
    }

    // Nothing chosen yet (a freshly added effect): the first cue layer - the
    // one the Layer combo shows. Without this the combo displayed a layer
    // the effect wasn't actually reading, so it output Off Value forever.
    // writeToJson() pins whatever this resolves to.
    if(m_layerId.isEmpty() && m_legacyLayerName.isEmpty() && !seq->cueLayers().isEmpty())
        return seq->cueLayers().first();

    return nullptr;
}

double CueMarkerEffect::computeValue(double t_globalTime) const
{
    CueLayer *layer = resolveLayer();
    if(!layer)
        return m_offValue;

    // Assumed sorted ascending, same as every other marker/beat consumer in this
    // codebase (CueLayer::insertMarker() maintains this; addMarkers()/
    // replaceMarkers() callers are expected to sort() afterward).
    const QList<float> &markers = layer->markers();
    if(markers.isEmpty())
        return m_offValue;

    // First marker strictly after this time; stepping back one gives the most
    // recent marker at or before it.
    auto it = std::upper_bound(markers.cbegin(), markers.cend(), static_cast<float>(t_globalTime));
    if(it == markers.cbegin())
        return m_offValue;   // before the very first marker

    const double markerTime = *(--it);
    const double elapsed = t_globalTime - markerTime;

    if(elapsed <= m_holdDuration)
        return m_onValue;

    if(m_falloffDuration > 0.0 && elapsed <= m_holdDuration + m_falloffDuration)
    {
        const double progress = (elapsed - m_holdDuration) / m_falloffDuration;
        const double eased = m_falloffCurve.valueForProgress(progress);
        return m_onValue + (m_offValue - m_onValue) * eased;
    }

    return m_offValue;
}

float *CueMarkerEffect::process(float *value, uint size, double t_time) const
{
    const float outputValue = static_cast<float>(computeValue(t_time + channel()->startTime()));
    for(uint i = 0; i < size; ++i)
        value[i] = outputValue;
    return value;
}

ChannelEffectEditor *CueMarkerEffect::createEditor()
{
    return new CueMarkerEffectEditor(this);
}

QWidget *CueMarkerEffect::createPropertyEditor()
{
    auto *form = new PropertyForm;

    // The layer list isn't static like the other combos here, so it can't go
    // through PropertyWidgets::createOptions (which bakes its items in once) -
    // built and kept live the same way CueMarkerEffectEditor::refreshLayerCombo()
    // does, just re-hosted as a PropertyForm row instead of a curve-editor one.
    auto *layerCombo = new QComboBox;
    auto refreshLayerCombo = [this, layerCombo]() {
        CueLayer *current = resolveLayer();
        layerCombo->blockSignals(true);
        layerCombo->clear();
        if (Channel *ch = channel()) {
            if (Sequence *seq = ch->sequence()) {
                for (auto *layer : seq->cueLayers())
                    layerCombo->addItem(layer->name(), QString::fromUtf8(layer->uniqueId()));
            }
        }
        if(current)
            layerCombo->setCurrentIndex(layerCombo->findData(QString::fromUtf8(current->uniqueId())));
        layerCombo->blockSignals(false);
    };
    refreshLayerCombo();
    QObject::connect(layerCombo, &QComboBox::currentIndexChanged, layerCombo, [this, layerCombo](int index){
        if (index < 0)
            return;
        setLayerId(layerCombo->itemData(index).toString().toUtf8());
    });
    // Connected on layerCombo, so this - and the sequence connections below -
    // disconnect automatically once the page is closed/replaced.
    if (Channel *ch = channel()) {
        if (Sequence *seq = ch->sequence()) {
            QObject::connect(seq, &Sequence::cueLayerAdded, layerCombo, [refreshLayerCombo](CueLayer*){ refreshLayerCombo(); });
            QObject::connect(seq, &Sequence::cueLayerRemoved, layerCombo, [refreshLayerCombo](CueLayer*){ refreshLayerCombo(); });
        }
    }
    form->addRow("Layer", layerCombo);

    form->addRow("On Value", PropertyWidgets::createNumber(m_onValue,
        {{PropertyWidgets::MetaMinimum, -10000.0}, {PropertyWidgets::MetaMaximum, 10000.0}, {PropertyWidgets::MetaSoftMinimum, 0.0}, {PropertyWidgets::MetaSoftMaximum, 1.0}},
        [this](double v){ setOnValue(v); }));
    form->addRow("Off Value", PropertyWidgets::createNumber(m_offValue,
        {{PropertyWidgets::MetaMinimum, -10000.0}, {PropertyWidgets::MetaMaximum, 10000.0}, {PropertyWidgets::MetaSoftMinimum, 0.0}, {PropertyWidgets::MetaSoftMaximum, 1.0}},
        [this](double v){ setOffValue(v); }));
    form->addRow("Hold Duration", PropertyWidgets::createNumber(m_holdDuration,
        {{PropertyWidgets::MetaMinimum, 0.0}, {PropertyWidgets::MetaMaximum, 9999.0}, {PropertyWidgets::MetaSoftMinimum, 0.0}, {PropertyWidgets::MetaSoftMaximum, 5.0}},
        [this](double v){ setHoldDuration(v); }));
    // Same list-index-equals-QEasingCurve::Type-value assumption as
    // CueMarkerEffectEditor's own falloff combo.
    form->addRow("Falloff Type", PropertyWidgets::createOptions(easeStrings(), m_falloffType, {},
        [this](int v){ setFalloffType(static_cast<QEasingCurve::Type>(v)); }));
    form->addRow("Falloff Duration", PropertyWidgets::createNumber(m_falloffDuration,
        {{PropertyWidgets::MetaMinimum, 0.0}, {PropertyWidgets::MetaMaximum, 9999.0}, {PropertyWidgets::MetaSoftMinimum, 0.0}, {PropertyWidgets::MetaSoftMaximum, 5.0}},
        [this](double v){ setFalloffDuration(v); }));

    return form;
}

void CueMarkerEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("layerId"))
        m_layerId = t_json.value("layerId").toString().toUtf8();
    else if(t_json.contains("layerName"))
        m_legacyLayerName = t_json.value("layerName").toString();
    m_onValue = t_json.value("onValue").toDouble(m_onValue);
    m_offValue = t_json.value("offValue").toDouble(m_offValue);
    m_holdDuration = std::max(t_json.value("holdDuration").toDouble(m_holdDuration), 0.0);
    m_falloffType = static_cast<QEasingCurve::Type>(t_json.value("falloffType").toInt(m_falloffType));
    m_falloffCurve.setType(m_falloffType);
    m_falloffDuration = std::max(t_json.value("falloffDuration").toDouble(m_falloffDuration), 0.0);
}

void CueMarkerEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    // The layer actually being read, so one picked by default stays put if
    // layers are added or reordered later.
    QByteArray layerId = m_layerId;
    if(layerId.isEmpty())
    {
        if(CueLayer *layer = resolveLayer())
            layerId = layer->uniqueId();
    }
    t_json.insert("layerId", QString::fromUtf8(layerId));
    t_json.insert("onValue", m_onValue);
    t_json.insert("offValue", m_offValue);
    t_json.insert("holdDuration", m_holdDuration);
    t_json.insert("falloffType", m_falloffType);
    t_json.insert("falloffDuration", m_falloffDuration);
}

} // namespace photon
