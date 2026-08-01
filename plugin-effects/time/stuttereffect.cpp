#include <QHBoxLayout>
#include <QLabel>
#include <qmath.h>
#include "view/numberscrubfield.h"
#include "stuttereffect.h"
#include "sequence/channel.h"
#include "sequence/clip.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "gui/gizmo/gizmohandle.h"


namespace photon {

StutterEffectEditor::StutterEffectEditor(StutterEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{

    keira::NumberScrubField *durationSpin = new keira::NumberScrubField;
    durationSpin->setMinimum(.001);   // a zero stutter duration divides by zero
    durationSpin->setValue(m_effect->duration());
    connect(durationSpin, &keira::NumberScrubField::valueChanged, this, &StutterEffectEditor::durationChanged);


    keira::NumberScrubField *gapSpin = new keira::NumberScrubField;
    gapSpin->setMinimum(0);
    gapSpin->setValue(m_effect->gap());
    connect(gapSpin, &keira::NumberScrubField::valueChanged, this, &StutterEffectEditor::gapChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(durationSpin, "Duration");
    paramWidget->addWidget(gapSpin, "Gap");

    addWidget(paramWidget, "Stutter");

    // Handles laid out along the time axis: origin, then the duration span, then
    // the gap span. Declared in data space; the group maps them to the view.
    m_gizmos = new GizmoGroup(scene(), this);

    m_originHandle = m_gizmos->addHandle();
    m_originHandle->setDataGetter([this]{ return QPointF(m_referenceTime, 0); });

    m_durationHandle = m_gizmos->addHandle(GizmoHandle::Anchor, Qt::Horizontal);
    m_durationHandle->setDataGetter([this]{
        return QPointF(m_referenceTime + m_effect->duration(), 0);
    });
    m_durationHandle->setDataSetter([this, durationSpin](QPointF pt){
        m_effect->setDuration(std::max(.001, pt.x() - m_referenceTime));
        durationSpin->setValue(m_effect->duration());
    });

    m_gapHandle = m_gizmos->addHandle(GizmoHandle::Anchor, Qt::Horizontal);
    m_gapHandle->setDataGetter([this]{
        return QPointF(m_referenceTime + m_effect->duration() + m_effect->gap(), 0);
    });
    m_gapHandle->setDataSetter([this, gapSpin](QPointF pt){
        m_effect->setGap(std::max(0.0, pt.x() - (m_referenceTime + m_effect->duration())));
        gapSpin->setValue(m_effect->gap());
    });

    m_gizmos->connectLine(m_originHandle, m_durationHandle);
    m_gizmos->connectLine(m_durationHandle, m_gapHandle);
}

void StutterEffectEditor::durationChanged(double t_value)
{
    m_effect->setDuration(t_value);
}

void StutterEffectEditor::gapChanged(double t_value)
{
    m_effect->setGap(t_value);
}

void StutterEffectEditor::relayout(const QRectF &t_sceneRect)
{
    // Anchor to the first stutter period (duration + gap) visible in the view.
    double period = m_effect->duration() + m_effect->gap();
    double startTime = m_effect->channel()->startTime();

    double x = startTime;
    if(t_sceneRect.left() > startTime && period > 0)
        x = (ceil((t_sceneRect.left() - startTime) / period) * period) + startTime;

    m_referenceTime = x;

    m_gizmos->setTransform(transform());
}



EffectInformation StutterEffect::info()
{
    EffectInformation toReturn([](){return new StutterEffect;});
    toReturn.name = "Stutter";
    toReturn.effectId = "photon.effect.stutter";
    toReturn.categories.append("Time");

    return toReturn;
}



StutterEffect::StutterEffect() : ChannelEffect()
{

}

void StutterEffect::setDuration(double t_value)
{
    m_duration = t_value;
    updated();

}

void StutterEffect::setGap(double t_value)
{
    m_gap = t_value;
    updated();
}

float * StutterEffect::process(float *value, uint size, double time) const
{

    if(previousEffect())
    {

        double period = m_duration + m_gap;
        double i = std::floor(time / period);
        double remainder = time -  (period * i);

        double offset = i * m_duration;
        //double offset = i * period;

        if(remainder < m_duration)
            offset += remainder;
        else
            offset += m_duration;

        return previousEffect()->process(value, size, offset);
    }


    return value;
}

ChannelEffectEditor *StutterEffect::createEditor()
{
    return new StutterEffectEditor(this);
}

void StutterEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("duration"))
        m_duration = t_json.value("duration").toDouble();
    if(t_json.contains("gap"))
        m_gap = t_json.value("gap").toDouble();
}

void StutterEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("duration", m_duration);
    t_json.insert("gap", m_gap);
}

} // namespace photon
