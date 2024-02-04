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

void KaleidoscopeClipEffect::evaluate(CanvasClipEffectEvaluationContext &t_context)
{
    //float w = static_cast<float>(t_context.canvasImage->width());
    //float h = static_cast<float>(t_context.canvasImage->height());

    double val1 = t_context.channelValues["value1"].toDouble();
    double val2 = t_context.channelValues["value2"].toDouble();

    if(!m_plane)
    {
        m_plane = new OpenGLPlane(t_context.openglContext, bounds_d{-1,-1,1,1}, false);
        m_shader = new OpenGLShader(t_context.openglContext, ":/resources/shader/BasicTextureVertex.vert",
                                    ":/clip-effect-resources/shader/distort/kaleidoscope.frag");

        m_basicShader = new OpenGLShader(t_context.openglContext, ":/resources/shader/BasicTextureVertex.vert",
                                    ":/resources/shader/texture.frag");
        m_shader->bind(t_context.openglContext);

        int w = t_context.canvas->width();
        int h = t_context.canvas->height();

        m_texture = new OpenGLTexture;
        m_texture->resize(t_context.openglContext, QImage::Format::Format_ARGB32_Premultiplied, w, h);
    }

    OpenGLFrameBuffer buffer(m_texture, t_context.openglContext);
    t_context.openglContext->functions()->glClearColor(.0f,.0f,.0f,.0f);
    t_context.openglContext->functions()->glClear(GL_COLOR_BUFFER_BIT);

    m_shader->bind(t_context.openglContext);
    m_shader->setTexture("tex",t_context.buffer->texture()->handle());
    m_shader->setFloat("value1",val1);
    m_shader->setFloat("value2",val2);

    m_plane->draw();

    t_context.buffer->bind();


    t_context.openglContext->functions()->glClearColor(.0f,.0f,.0f,.0f);
    t_context.openglContext->functions()->glClear(GL_COLOR_BUFFER_BIT);

    m_basicShader->bind(t_context.openglContext);
    m_basicShader->setTexture("tex",m_texture->handle());
    m_plane->draw();

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
