#include "kaleidoscopeclipeffect.h"
#include "opengl/openglshader.h"
#include "opengl/openglplane.h"
#include "opengl/opengltexture.h"
#include "pixel/canvas.h"
#include "channel/parameter/numberchannelparameter.h"

namespace photon {


KaleidoscopeClipEffect::KaleidoscopeClipEffect()
{

}

void KaleidoscopeClipEffect::init()
{

    addChannelParameter(new NumberChannelParameter("value1",2.0));
    addChannelParameter(new NumberChannelParameter("value2",.65));



}

void KaleidoscopeClipEffect::initializeContext(QOpenGLContext *t_context, Canvas *t_canvas)
{
    m_shader = new OpenGLShader(t_context, ":/resources/shader/BasicTextureVertex.vert",
                                ":/clip-effect-resources/shader/distort/kaleidoscope.frag");

    m_shader->bind(t_context);

    m_texture = new OpenGLTexture;
    m_texture->resize(t_context, QImage::Format::Format_ARGB32_Premultiplied, t_canvas->width(), t_canvas->height());

}

void KaleidoscopeClipEffect::canvasResized(QOpenGLContext *t_context, Canvas *t_canvas)
{
    m_texture->resize(t_context, QImage::Format::Format_ARGB32_Premultiplied, t_canvas->width(), t_canvas->height());
}

void KaleidoscopeClipEffect::evaluate(CanvasEffectEvaluationContext &t_context)
{
    //float w = static_cast<float>(t_context.canvasImage->width());
    //float h = static_cast<float>(t_context.canvasImage->height());

    double val1 = t_context.channelValues["value1"].toDouble();
    double val2 = t_context.channelValues["value2"].toDouble();


    OpenGLFrameBuffer buffer(m_texture, t_context.openglContext);
    t_context.openglContext->functions()->glClearColor(.0f,.0f,.0f,.0f);
    t_context.openglContext->functions()->glClear(GL_COLOR_BUFFER_BIT);

    m_shader->bind(t_context.openglContext);
    m_shader->setTexture("tex",t_context.buffer->texture()->handle());
    m_shader->setFloat("value1",val1);
    m_shader->setFloat("value2",val2);

    t_context.resources->drawPlane();

    t_context.buffer->bind();


    t_context.openglContext->functions()->glClearColor(.0f,.0f,.0f,.0f);
    t_context.openglContext->functions()->glClear(GL_COLOR_BUFFER_BIT);

    t_context.resources->bindBasicShader(m_texture->handle());
    t_context.resources->drawPlane();

}

ClipEffectInformation KaleidoscopeClipEffect::info()
{
    ClipEffectInformation toReturn([](){return new KaleidoscopeClipEffect;});
    toReturn.name = "Kaleidoscope";
    toReturn.id = "photon.clip.effect.Kaleidoscope";
    toReturn.categories.append("Distort");

    return toReturn;
}




} // namespace photon
