#include "palettegizmochanneleffect.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "sequence/channel.h"
#include "surface/surfacegizmocontainer.h"
#include "surface/palettegizmo.h"
#include "util/utils.h"

namespace photon {

PaletteGizmoChannelEffectEditor::PaletteGizmoChannelEffectEditor(PaletteGizmoChannelEffect *t_effect):GizmoChannelEffectEditor(t_effect)
{
    StackedParameterWidget *paramWidget = createParameterWidget(t_effect);

    addWidget(paramWidget, "Toggle Gizmo");

}

PaletteGizmoChannelEffect::PaletteGizmoChannelEffect() : GizmoChannelEffect(PaletteGizmo::GizmoId)
{

}

float * PaletteGizmoChannelEffect::process(float *value, uint size, double time) const
{


    if(gizmo())
    {
        PaletteGizmo *paletteGizmo = dynamic_cast<PaletteGizmo*>(gizmo());

        if(paletteGizmo->hasSelectedColor())
            return colorToValues(paletteGizmo->selectedColor(), value, size);
    }


    /*
    for(int i = 0; i < size; ++i)
    {
        value[i] = isPressed ? m_onValue : m_offValue;
    }
*/
    return value;
}

ChannelEffectEditor *PaletteGizmoChannelEffect::createEditor()
{
    return new PaletteGizmoChannelEffectEditor(this);
}

EffectInformation PaletteGizmoChannelEffect::info()
{
    EffectInformation toReturn([](){return new PaletteGizmoChannelEffect;});
    toReturn.name = "Palette Gizmo";
    toReturn.effectId = "photon.effect.palette-gizmo";
    toReturn.categories.append("Gizmo");

    return toReturn;
}

} // namespace photon
