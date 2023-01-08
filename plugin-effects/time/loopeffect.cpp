#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include "loopeffect.h"
#include "sequence/viewer/stackedparameterwidget.h"

namespace photon {

LoopEffectEditor::LoopEffectEditor(LoopEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    QDoubleSpinBox *durationSpin = new QDoubleSpinBox;
    durationSpin->setValue(m_effect->duration());
    connect(durationSpin, &QDoubleSpinBox::valueChanged, this, &LoopEffectEditor::durationChanged);


    QCheckBox *mirrorCheck = new QCheckBox;
    mirrorCheck->setChecked(m_effect->mirror());
    connect(mirrorCheck, &QCheckBox::toggled, this, &LoopEffectEditor::mirrorChanged);


    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(durationSpin, "Duration");
    paramWidget->addWidget(mirrorCheck, "Mirror");

    addWidget(paramWidget, "Sinewave");
}

void LoopEffectEditor::durationChanged(double t_value)
{
    m_effect->setDuration(t_value);
}

void LoopEffectEditor::mirrorChanged(bool t_value)
{
    m_effect->setMirror(t_value);
}




EffectInformation LoopEffect::info()
{
    EffectInformation toReturn([](){return new LoopEffect;});
    toReturn.name = "Loop";
    toReturn.effectId = "photon.effect.mirror";
    toReturn.categories.append("Time");

    return toReturn;
}



LoopEffect::LoopEffect() : ChannelEffect()
{

}

void LoopEffect::setDuration(double t_value)
{
    m_duration = t_value;
    updated();

}

void LoopEffect::setMirror(bool t_value)
{
    m_mirror = t_value;
    updated();
}

double LoopEffect::process(double value, double time) const
{
    if(previousEffect())
    {
        double t = 0;

        if(m_duration > 0)
            t = time / m_duration;

        double floored = std::floor(t);
        double loopedTime = time - (m_duration * floored);

        if(m_mirror && std::remainder(floored+1, 2.0) == 0)
        {
            loopedTime = m_duration - loopedTime;
        }


        value = previousEffect()->process(value, loopedTime);
    }
    return value;
}

ChannelEffectEditor *LoopEffect::createEditor()
{
    return new LoopEffectEditor(this);
}

void LoopEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("duration"))
        m_duration = t_json.value("duration").toDouble();
    if(t_json.contains("mirror"))
        m_mirror = t_json.value("mirror").toBool();
}

void LoopEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("duration", m_duration);
    t_json.insert("mirror", m_mirror);
}

} // namespace photon
