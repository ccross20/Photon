#include <Qimage>
#include "fillcoloreffect.h"
#include "channel/parameter/colorchannelparameter.h"

namespace photon {

FillColorEffect::FillColorEffect()
{

}

void FillColorEffect::init()
{
    addChannelParameter(new ColorChannelParameter("color", Qt::white));

}

void FillColorEffect::evaluate(CanvasClipEffectEvaluationContext &t_context) const
{
    QColor color = t_context.channelValues["color"].value<QColor>();
    color.setAlphaF(t_context.strength);
    t_context.canvasImage->fill(color);

}

ClipEffectInformation FillColorEffect::info()
{
    ClipEffectInformation toReturn([](){return new FillColorEffect;});
    toReturn.name = "Fill Color";
    toReturn.id = "photon.clip.effect.fill-color";
    toReturn.categories.append("Fill");

    return toReturn;
}

} // namespace photon
