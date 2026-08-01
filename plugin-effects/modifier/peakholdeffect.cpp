#include <QComboBox>
#include <algorithm>
#include <cmath>
#include <limits>
#include "peakholdeffect.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "view/numberscrubfield.h"

namespace photon {

PeakHoldEffectEditor::PeakHoldEffectEditor(PeakHoldEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    auto *thresholdSpin = new keira::NumberScrubField;
    thresholdSpin->setValue(m_effect->threshold());
    connect(thresholdSpin, &keira::NumberScrubField::valueChanged, this, &PeakHoldEffectEditor::thresholdChanged);

    auto *holdSpin = new keira::NumberScrubField;
    holdSpin->setMinimum(0.0);
    holdSpin->setMaximum(9999);
    holdSpin->setValue(m_effect->holdTime());
    connect(holdSpin, &keira::NumberScrubField::valueChanged, this, &PeakHoldEffectEditor::holdTimeChanged);

    auto *decaySpin = new keira::NumberScrubField;
    decaySpin->setMinimum(0.0);
    decaySpin->setMaximum(9999);
    decaySpin->setValue(m_effect->decayTime());
    connect(decaySpin, &keira::NumberScrubField::valueChanged, this, &PeakHoldEffectEditor::decayTimeChanged);

    // Same full QEasingCurve::Type list (and list-index-equals-enum-value
    // assumption) as EaseEffectEditor, so setCurrentIndex/the enum cast below
    // stay in sync without a separate index<->type mapping table.
    QStringList curveStrings;
    curveStrings << "Linear";
    curveStrings << "In Quad" << "Out Quad" << "In Out Quad" << "Out In Quad";
    curveStrings << "In Cubic" << "Out Cubic" << "In Out Cubic" << "Out In Cubic";
    curveStrings << "In Quart" << "Out Quart" << "In Out Quart" << "Out In Quart";
    curveStrings << "In Quint" << "Out Quint" << "In Out Quint" << "Out In Quint";
    curveStrings << "In Sine" << "Out Sine" << "In Out Sine" << "Out In Sine";
    curveStrings << "In Expo" << "Out Expo" << "In Out Expo" << "Out In Expo";
    curveStrings << "In Circ" << "Out Circ" << "In Out Circ" << "Out In Circ";
    curveStrings << "In Elastic" << "Out Elastic" << "In Out Elastic" << "Out In Elastic";
    curveStrings << "In Back" << "Out Back" << "In Out Back" << "Out In Back";
    curveStrings << "In Bounce" << "Out Bounce" << "In Out Bounce" << "Out In Bounce";

    QComboBox *curveCombo = new QComboBox;
    curveCombo->addItems(curveStrings);
    curveCombo->setCurrentIndex(m_effect->decayCurveType());
    connect(curveCombo, &QComboBox::currentIndexChanged, this, &PeakHoldEffectEditor::decayCurveChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(thresholdSpin, "Threshold");
    paramWidget->addWidget(holdSpin, "Hold Time");
    paramWidget->addWidget(decaySpin, "Decay Time");
    paramWidget->addWidget(curveCombo, "Decay Curve");

    addWidget(paramWidget, "Peak Hold");
}

void PeakHoldEffectEditor::thresholdChanged(double t_value)
{
    m_effect->setThreshold(t_value);
}

void PeakHoldEffectEditor::holdTimeChanged(double t_value)
{
    m_effect->setHoldTime(t_value);
}

void PeakHoldEffectEditor::decayTimeChanged(double t_value)
{
    m_effect->setDecayTime(t_value);
}

void PeakHoldEffectEditor::decayCurveChanged(int t_value)
{
    m_effect->setDecayCurveType(static_cast<QEasingCurve::Type>(t_value));
}

EffectInformation PeakHoldEffect::info()
{
    EffectInformation toReturn([](){return new PeakHoldEffect;});
    toReturn.name = "Peak Hold";
    toReturn.effectId = "photon.effect.peak-hold";
    toReturn.categories.append("Modifier");

    return toReturn;
}

PeakHoldEffect::PeakHoldEffect() : ChannelEffect()
{
    m_decayCurve.setType(m_decayCurveType);
}

void PeakHoldEffect::setThreshold(double t_value)
{
    m_threshold = t_value;
    updated();
}

void PeakHoldEffect::setHoldTime(double t_value)
{
    m_holdTime = std::max(t_value, 0.0);
    updated();
}

void PeakHoldEffect::setDecayTime(double t_value)
{
    m_decayTime = std::max(t_value, 0.0);
    updated();
}

void PeakHoldEffect::setDecayCurveType(QEasingCurve::Type t_value)
{
    m_decayCurveType = t_value;
    m_decayCurve.setType(t_value);
    updated();
}

float *PeakHoldEffect::process(float *value, uint size, double time) const
{
    if(previousEffect())
        value = previousEffect()->process(value, size, time);

    const double window = m_holdTime + m_decayTime;
    if(window <= 0.0 || !previousEffect())
        return value;

    // Bounded backward scan for the most recent upward threshold crossing within
    // the last `window` seconds - see the class comment for why this can't just
    // track running state like a real envelope follower would.
    constexpr int maxScanSteps = 150;
    const double step = std::max(window / maxScanSteps, 0.005);

    float prevSample = 0.0f;
    bool havePrev = false;
    double triggerTime = -std::numeric_limits<double>::infinity();
    float peakValue = 0.0f;

    for(double t = time; t >= time - window; t -= step)
    {
        float sample[1] = {0.0f};
        previousEffect()->process(sample, 1, t);

        if(havePrev && sample[0] < m_threshold && prevSample >= m_threshold)
        {
            // Walking backward, this is the step just past the crossing - close
            // enough given hold/decay times are normally well above the scan step.
            triggerTime = t + step;
            peakValue = prevSample;
            break;
        }
        prevSample = sample[0];
        havePrev = true;
    }

    float envelope = 0.0f;
    if(std::isfinite(triggerTime))
    {
        const double elapsed = time - triggerTime;
        if(elapsed <= m_holdTime)
            envelope = 1.0f;
        else if(m_decayTime > 0.0 && elapsed <= window)
            envelope = static_cast<float>(m_decayCurve.valueForProgress(1.0 - (elapsed - m_holdTime) / m_decayTime));
    }

    const float outValue = peakValue * envelope;
    for(int i = 0; i < size; ++i)
        value[i] = outValue;

    return value;
}

ChannelEffectEditor *PeakHoldEffect::createEditor()
{
    return new PeakHoldEffectEditor(this);
}

void PeakHoldEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("threshold"))
        m_threshold = t_json.value("threshold").toDouble();
    if(t_json.contains("holdTime"))
        m_holdTime = t_json.value("holdTime").toDouble();
    if(t_json.contains("decayTime"))
        m_decayTime = t_json.value("decayTime").toDouble();
    if(t_json.contains("decayCurveType"))
    {
        m_decayCurveType = static_cast<QEasingCurve::Type>(t_json.value("decayCurveType").toInt());
        m_decayCurve.setType(m_decayCurveType);
    }
}

void PeakHoldEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("threshold", m_threshold);
    t_json.insert("holdTime", m_holdTime);
    t_json.insert("decayTime", m_decayTime);
    t_json.insert("decayCurveType", m_decayCurveType);
}

} // namespace photon
