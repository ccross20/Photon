#include <QPen>
#include <QDoubleSpinBox>
#include "gizmoretimechanneleffect.h"
#include "sequence/channel.h"
#include "surface/surfacegizmocontainer.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "surface/togglegizmo.h"

namespace photon {

GizmoRetimeChannelEffectEditor::GizmoRetimeChannelEffectEditor(GizmoRetimeChannelEffect *t_effect):GizmoChannelEffectEditor(t_effect),m_effect(t_effect)
{
    //setMaximumHeight(40);
    StackedParameterWidget *paramWidget = createParameterWidget(t_effect);


    addWidget(paramWidget, "Toggle Gizmo");
}


EffectInformation GizmoRetimeChannelEffect::info()
{
    EffectInformation toReturn([](){return new GizmoRetimeChannelEffect;});
    toReturn.name = "Gizmo Retime";
    toReturn.effectId = "photon.effect.gizmo-retime";
    toReturn.categories.append("Gizmo");

    return toReturn;
}



GizmoRetimeChannelEffect::GizmoRetimeChannelEffect() : GizmoChannelEffect(ToggleGizmo::GizmoId)
{

}


void GizmoRetimeChannelEffect::addedToChannel()
{
    GizmoChannelEffect::addedToChannel();
    if(!m_container)
        m_container = channel()->gizmoContainer();
}

float * GizmoRetimeChannelEffect::process(float *value, uint size, double time) const
{
    if(previousEffect())
    {
        return previousEffect()->process(value, size, time);
    }

    return value;
}

double GizmoRetimeChannelEffect::processTime(double t_time) const
{
    if(!m_container)
        return t_time;


    if(gizmo())
    {
        ToggleGizmo *toggleGizmo = dynamic_cast<ToggleGizmo*>(gizmo());
        //qDebug() <<  toggleGizmo->isPressed();
        return toggleGizmo->timeSincePress();
    }

    return t_time;
}

ChannelEffectEditor *GizmoRetimeChannelEffect::createEditor()
{
    return new GizmoRetimeChannelEffectEditor(this);
}

void GizmoRetimeChannelEffect::readFromJson(const QJsonObject &t_json)
{
    GizmoChannelEffect::readFromJson(t_json);
}

void GizmoRetimeChannelEffect::writeToJson(QJsonObject &t_json) const
{
    GizmoChannelEffect::writeToJson(t_json);
}

} // namespace photon

