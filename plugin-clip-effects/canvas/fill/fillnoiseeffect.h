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
    void initializeContext(QOpenGLContext *, Canvas *) override;
    void canvasResized(QOpenGLContext *, Canvas *) override;
    void evaluate(CanvasClipEffectEvaluationContext &) override;
    static ClipEffectInformation info();

private:
    NoiseGenerator m_noise;
    OpenGLShader *m_shader = nullptr;
    OpenGLPlane *m_plane = nullptr;
    OpenGLTexture *m_texture = nullptr;
};

} // namespace photon

#endif // PHOTON_FILLNOISEEFFECT_H
