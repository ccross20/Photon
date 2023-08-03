#ifndef PHOTON_DRAWELLIPSECLIPEFFECT_H
#define PHOTON_DRAWELLIPSECLIPEFFECT_H

#include <QWidget>
#include "sequence/canvasclipeffect.h"

namespace photon {

class DrawEllipseClipEffect : public CanvasClipEffect
{
public:
    DrawEllipseClipEffect();

    void init() override;
    void evaluate(CanvasClipEffectEvaluationContext &) const override;
    static ClipEffectInformation info();
};

} // namespace photon

#endif // PHOTON_DRAWELLIPSECLIPEFFECT_H
