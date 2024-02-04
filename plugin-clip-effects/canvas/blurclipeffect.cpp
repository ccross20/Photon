#include "blurclipeffect.h"
#include "channel/parameter/numberchannelparameter.h"
#include "opengl/openglshader.h"
#include "opengl/openglplane.h"
#include "opengl/opengltexture.h"
#include "opengl/openglframebuffer.h"
#include "photoncore.h"
#include "pixel/canvas.h"

namespace photon {


BlurClipEffect::BlurClipEffect()
{

}

void BlurClipEffect::init()
{
    addChannelParameter(new NumberChannelParameter("strength",10.0));

    QOffscreenSurface *surface = photonApp->surface();

    QOpenGLContext context;

    context.setShareContext(QOpenGLContext::globalShareContext());
    context.create();
    context.makeCurrent(surface);




}

void BlurClipEffect::evaluate(CanvasClipEffectEvaluationContext &t_context)
{
    //float w = static_cast<float>(t_context.canvasImage->width());
    //float h = static_cast<float>(t_context.canvasImage->height());

    double amount = t_context.channelValues["strength"].toDouble();

    if(amount <= 0.0)
        return;


    if(!m_plane)
    {
        m_plane = new OpenGLPlane(t_context.openglContext, bounds_d{-1,-1,1,1}, true);
        m_shader = new OpenGLShader(t_context.openglContext, ":/resources/shader/BasicTextureVertex.vert",
                                    ":/clip-effect-resources/shader/gauss_blur.frag");
        m_outputShader = new OpenGLShader(t_context.openglContext, ":/resources/shader/BasicTextureVertex.vert",
                                    ":/resources/shader/texture.frag");
        m_centerShader = new OpenGLShader(t_context.openglContext, ":/resources/shader/BasicTextureVertex.vert",
                                    ":/resources/shader/texture.frag");
    }


    int w = t_context.canvas->width();
    int h = t_context.canvas->height();

    auto tex = new OpenGLPingPongTexture;
    tex->resize(t_context.openglContext, QImage::Format::Format_ARGB32_Premultiplied, w, h);

    auto inputTexture = t_context.buffer->texture();

    OpenGLFrameBuffer buffer(tex->destination(), t_context.openglContext);

    m_centerShader->bind(t_context.openglContext);
    m_centerShader->setFloat2("scale", (inputTexture->width() / static_cast<double>(w)), (inputTexture->height() / static_cast<double>(h)));


    m_centerShader->setTexture("tex",inputTexture->handle());
    m_plane->draw();
    tex->swap();
    buffer.setTexture(tex->destination());


    m_shader->bind(t_context.openglContext);
    m_shader->setTexture("tex",tex->source()->handle());
    m_shader->setInt("radius", amount);
    m_shader->setFloat("sigma",  amount * .5);
    m_shader->setFloat2("wh_rcp", 1.f/w, 1.f/h);
    m_shader->setFloat2("dir",1,0);

    m_plane->draw();
    tex->swap();
    buffer.setTexture(tex->destination());
    t_context.openglContext->functions()->glClearColor(.0f,.0f,.0f,.0f);
    t_context.openglContext->functions()->glClear(GL_COLOR_BUFFER_BIT);

    m_shader->setTexture("tex",tex->source()->handle());
    m_shader->setFloat2("dir",0,1);
    m_plane->draw();


    buffer.unbind();

    t_context.buffer->bind();
    tex->swap();
    t_context.openglContext->functions()->glClearColor(0,0,0,0);
    t_context.openglContext->functions()->glClear(GL_COLOR_BUFFER_BIT);
    m_outputShader->setTexture("tex",tex->destination()->handle());
    m_plane->draw();

    tex->destroy();
    delete tex;

}

ClipEffectInformation BlurClipEffect::info()
{
    ClipEffectInformation toReturn([](){return new BlurClipEffect;});
    toReturn.name = "Blur";
    toReturn.id = "photon.clip.effect.blur";
    toReturn.categories.append("Blur");

    return toReturn;
}

} // namespace photon
