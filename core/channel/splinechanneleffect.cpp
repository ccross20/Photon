#include <QMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "splinechanneleffect.h"
#include "sequence/channel.h"

namespace photon {

SplineEffectEditor::SplineEffectEditor(SplineChannelEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{

}

void SplineEffectEditor::mousePressEvent(QMouseEvent *event)
{
    ChannelEffectEditor::mousePressEvent(event);

    if(event->modifiers() & Qt::ControlModifier)
    {
        auto mappedPt = transform().inverted().map(event->pos().toPointF());

        auto spline = m_effect->value();

        mappedPt.setX(mappedPt.x() - m_effect->channel()->startTime());

        spline.insert(SplinePoint{mappedPt});
        m_effect->setValue(spline);
    }
}




SplineChannelEffect::SplineChannelEffect(): ChannelEffect()
{

}

Spline SplineChannelEffect::value() const
{
    return m_spline;
}

void SplineChannelEffect::setValue(const Spline &t_spline)
{
    m_spline = t_spline;
    updated();
}

float * SplineChannelEffect::process(float *value, uint size, double time) const
{
    auto val = m_spline.value(time, true);
    for(int i = 0; i < size; ++i)
    {
        value[i] = val;
    }
    return value;
}

ChannelEffectEditor *SplineChannelEffect::createEditor()
{
    return new SplineEffectEditor(this);
}

void SplineChannelEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
}

void SplineChannelEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
}

EffectInformation SplineChannelEffect::info()
{
    EffectInformation toReturn([](){return new SplineChannelEffect;});
    toReturn.name = "Spline";
    toReturn.effectId = "photon.effect.spline";
    toReturn.categories.append("Generator");

    return toReturn;
}


} // namespace photon
