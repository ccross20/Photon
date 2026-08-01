#include <cmath>
#include <QComboBox>
#include "channelremapeffect.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "view/numberscrubfield.h"
#include "util/utils.h"

namespace photon {

ChannelRemapEffectEditor::ChannelRemapEffectEditor(ChannelRemapEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    auto *minInSpin = new keira::NumberScrubField;
    minInSpin->setValue(m_effect->minInput());
    connect(minInSpin, &keira::NumberScrubField::valueChanged, this, &ChannelRemapEffectEditor::minInputChanged);

    auto *maxInSpin = new keira::NumberScrubField;
    maxInSpin->setValue(m_effect->maxInput());
    connect(maxInSpin, &keira::NumberScrubField::valueChanged, this, &ChannelRemapEffectEditor::maxInputChanged);

    auto *minOutSpin = new keira::NumberScrubField;
    minOutSpin->setValue(m_effect->minOutput());
    connect(minOutSpin, &keira::NumberScrubField::valueChanged, this, &ChannelRemapEffectEditor::minOutputChanged);

    auto *maxOutSpin = new keira::NumberScrubField;
    maxOutSpin->setValue(m_effect->maxOutput());
    connect(maxOutSpin, &keira::NumberScrubField::valueChanged, this, &ChannelRemapEffectEditor::maxOutputChanged);

    QComboBox *easeCombo = new QComboBox;
    easeCombo->addItems(easeStrings());
    easeCombo->setCurrentIndex(m_effect->easeType());
    connect(easeCombo, &QComboBox::currentIndexChanged, this, &ChannelRemapEffectEditor::easeTypeChanged);

    QComboBox *boundsCombo = new QComboBox;
    boundsCombo->addItems({"Limit","Loop","Ping Pong"});
    boundsCombo->setCurrentIndex(m_effect->boundsMode());
    connect(boundsCombo, &QComboBox::currentIndexChanged, this, &ChannelRemapEffectEditor::boundsModeChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(minInSpin, "Min Input");
    paramWidget->addWidget(maxInSpin, "Max Input");
    paramWidget->addWidget(easeCombo, "Ease Type");
    paramWidget->addWidget(minOutSpin, "Min Output");
    paramWidget->addWidget(maxOutSpin, "Max Output");
    paramWidget->addWidget(boundsCombo, "Bounds Mode");

    addWidget(paramWidget, "Remap");
}

void ChannelRemapEffectEditor::minInputChanged(double t_value)
{
    m_effect->setMinInput(t_value);
}

void ChannelRemapEffectEditor::maxInputChanged(double t_value)
{
    m_effect->setMaxInput(t_value);
}

void ChannelRemapEffectEditor::minOutputChanged(double t_value)
{
    m_effect->setMinOutput(t_value);
}

void ChannelRemapEffectEditor::maxOutputChanged(double t_value)
{
    m_effect->setMaxOutput(t_value);
}

void ChannelRemapEffectEditor::easeTypeChanged(int t_value)
{
    m_effect->setEaseType(static_cast<QEasingCurve::Type>(t_value));
}

void ChannelRemapEffectEditor::boundsModeChanged(int t_value)
{
    m_effect->setBoundsMode(static_cast<ChannelRemapEffect::BoundsMode>(t_value));
}

EffectInformation ChannelRemapEffect::info()
{
    EffectInformation toReturn([](){return new ChannelRemapEffect;});
    toReturn.name = "Remap";
    toReturn.effectId = "photon.effect.remap";
    toReturn.categories.append("Modifier");

    return toReturn;
}

ChannelRemapEffect::ChannelRemapEffect() : ChannelEffect()
{
    m_easing.setType(m_easeType);
}

void ChannelRemapEffect::setMinInput(double t_value)
{
    m_minInput = t_value;
    updated();
}

void ChannelRemapEffect::setMaxInput(double t_value)
{
    m_maxInput = t_value;
    updated();
}

void ChannelRemapEffect::setMinOutput(double t_value)
{
    m_minOutput = t_value;
    updated();
}

void ChannelRemapEffect::setMaxOutput(double t_value)
{
    m_maxOutput = t_value;
    updated();
}

void ChannelRemapEffect::setEaseType(QEasingCurve::Type t_value)
{
    m_easeType = t_value;
    m_easing.setType(t_value);
    updated();
}

void ChannelRemapEffect::setBoundsMode(BoundsMode t_value)
{
    m_boundsMode = t_value;
    updated();
}

float *ChannelRemapEffect::process(float *value, uint size, double time) const
{
    if(previousEffect())
        value = previousEffect()->process(value, size, time);

    const double inDelta = m_maxInput - m_minInput;
    const double outDelta = m_maxOutput - m_minOutput;

    for(int i = 0; i < size; ++i)
    {
        double inputValue = value[i];

        // Same algorithm as RemapValueNode (photon.math.remap) - kept in lockstep
        // so this effect and that node behave identically for the same settings.
        // Unlike the node, guard the degenerate minInput==maxInput case: dividing
        // by zero there would hand NaN/Inf downstream into the curve editor's
        // grid rendering, which has already crashed once this way (see the
        // BeatIntegerEffect out-of-bounds fix) - treat a zero-width input range
        // as "always at the low end" instead.
        double mappedIn = inDelta != 0.0 ? (inputValue - m_minInput) / inDelta : 0.0;

        switch(m_boundsMode)
        {
        default:
        case BoundsLimit:
            if(inputValue > m_maxInput)
                inputValue = m_maxInput;
            if(inputValue < m_minInput)
                inputValue = m_minInput;
            mappedIn = inDelta != 0.0 ? (inputValue - m_minInput) / inDelta : 0.0;
            break;
        case BoundsLoop:
            mappedIn = mappedIn - std::floor(mappedIn);
            break;
        case BoundsPingPong:
        {
            const double wrapped = std::fmod(mappedIn, 2.0);
            // std::abs(wrapped - 1.0) flips the second half of the cycle.
            mappedIn = 1.0 - std::abs(wrapped - 1.0);
        }
            break;
        }

        const double output = (m_easing.valueForProgress(mappedIn) * outDelta) + m_minOutput;
        value[i] = static_cast<float>(output);
    }

    return value;
}

ChannelEffectEditor *ChannelRemapEffect::createEditor()
{
    return new ChannelRemapEffectEditor(this);
}

void ChannelRemapEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("minInput"))
        m_minInput = t_json.value("minInput").toDouble();
    if(t_json.contains("maxInput"))
        m_maxInput = t_json.value("maxInput").toDouble();
    if(t_json.contains("minOutput"))
        m_minOutput = t_json.value("minOutput").toDouble();
    if(t_json.contains("maxOutput"))
        m_maxOutput = t_json.value("maxOutput").toDouble();
    if(t_json.contains("easeType"))
    {
        m_easeType = static_cast<QEasingCurve::Type>(t_json.value("easeType").toInt());
        m_easing.setType(m_easeType);
    }
    if(t_json.contains("boundsMode"))
        m_boundsMode = static_cast<BoundsMode>(t_json.value("boundsMode").toInt());
}

void ChannelRemapEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("minInput", m_minInput);
    t_json.insert("maxInput", m_maxInput);
    t_json.insert("minOutput", m_minOutput);
    t_json.insert("maxOutput", m_maxOutput);
    t_json.insert("easeType", m_easeType);
    t_json.insert("boundsMode", m_boundsMode);
}

} // namespace photon
