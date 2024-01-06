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
    t_context.canvasImage->fill(t_context.channelValues["color"].value<QColor>());

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
