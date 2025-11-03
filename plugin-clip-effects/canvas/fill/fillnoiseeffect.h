#ifndef PHOTON_FILLNOISEEFFECT_H
#define PHOTON_FILLNOISEEFFECT_H

#include <QWidget>
#include "sequence/canvaseffect.h"
#include "util/noisegenerator.h"

namespace photon {

class FillNoiseEffect : public CanvasEffect
{
public:
    FillNoiseEffect();

    void init() override;
    void initializeContext(QOpenGLContext *, Canvas *) override;
    void canvasResized(QOpenGLContext *, Canvas *) override;
    void evaluate(CanvasEffectEvaluationContext &) override;
    static ClipEffectInformation info();

private:
    NoiseGenerator m_noise;
    OpenGLShader *m_shader = nullptr;
    OpenGLPlane *m_plane = nullptr;
    OpenGLTexture *m_texture = nullptr;
};

} // namespace photon

#endif // PHOTON_FILLNOISEEFFECT_H
