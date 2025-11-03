#ifndef PHOTON_FILLCOLOREFFECT_H
#define PHOTON_FILLCOLOREFFECT_H

#include "sequence/canvaseffect.h"

namespace photon {

class FillColorEffect : public CanvasEffect
{
public:
    FillColorEffect();

    void init() override;
    void evaluate(CanvasEffectEvaluationContext &) override;
    static ClipEffectInformation info();
};

} // namespace photon

#endif // PHOTON_FILLCOLOREFFECT_H
