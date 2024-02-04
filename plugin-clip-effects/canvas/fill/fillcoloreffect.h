#ifndef PHOTON_FILLCOLOREFFECT_H
#define PHOTON_FILLCOLOREFFECT_H

#include "sequence/canvasclipeffect.h"

namespace photon {

class FillColorEffect : public CanvasClipEffect
{
public:
    FillColorEffect();

    void init() override;
    void evaluate(CanvasClipEffectEvaluationContext &) override;
    static ClipEffectInformation info();
};

} // namespace photon

#endif // PHOTON_FILLCOLOREFFECT_H
