#ifndef PHOTON_FILLNOISEEFFECT_H
#define PHOTON_FILLNOISEEFFECT_H

#include <QWidget>
#include "sequence/canvasclipeffect.h"
#include "util/noisegenerator.h"

namespace photon {

class FillNoiseEffect : public CanvasClipEffect
{
public:
    FillNoiseEffect();

    void init() override;
    void evaluate(CanvasClipEffectEvaluationContext &) override;
    static ClipEffectInformation info();

private:
    NoiseGenerator m_noise;
};

} // namespace photon

#endif // PHOTON_FILLNOISEEFFECT_H
