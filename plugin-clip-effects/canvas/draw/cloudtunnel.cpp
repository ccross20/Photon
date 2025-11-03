#include "cloudtunnel.h"
#include "opengl/openglshader.h"
#include "opengl/openglplane.h"
#include "pixel/canvas.h"
#include "channel/parameter/point2channelparameter.h"

namespace photon {


CloudTunnel::CloudTunnel()
{

}

void CloudTunnel::init()
{
    addChannelParameter(new Point2ChannelParameter("center",QPointF{.5,.5}));
 /*
    QOffscreenSurface *surface = photonApp->surface();

    QOpenGLContext context;

    context.setShareContext(QOpenGLContext::globalShareContext());
    context.create();
    context.makeCurrent(surface);
    */



}

void CloudTunnel::initializeContext(QOpenGLContext *t_context, Canvas *)
{
    m_plane = new OpenGLPlane(t_context, bounds_d{-1,-1,1,1}, false);
    m_shader = new OpenGLShader(t_context, ":/resources/shader/BasicTextureVertex.vert",
                                ":/clip-effect-resources/shader/generate/cloudtunnel.frag");
    m_shader->bind(t_context);
}

void CloudTunnel::canvasResized(QOpenGLContext *, Canvas *)
{

}

void CloudTunnel::evaluate(CanvasEffectEvaluationContext &t_context)
{
    //float w = static_cast<float>(t_context.canvasImage->width());
    //float h = static_cast<float>(t_context.canvasImage->height());


    QPointF pos = t_context.channelValues["center"].value<QPointF>();


    t_context.openglContext->functions()->glClearColor(.0f,.0f,.0f,.0f);
    t_context.openglContext->functions()->glClear(GL_COLOR_BUFFER_BIT);

    m_shader->bind(t_context.openglContext);


    m_shader->setFloat2("iResolution", t_context.canvas->width(), t_context.canvas->height());
    //m_shader->setFloat("iTime", t_context.relativeTime);
    //m_shader->setFloat2("centerPt", pos.x(), pos.y());


    m_plane->draw(t_context.openglContext);
    m_shader->unbind();

}

ClipEffectInformation CloudTunnel::info()
{
    ClipEffectInformation toReturn([](){return new CloudTunnel;});
    toReturn.name = "Cloud Tunnel";
    toReturn.id = "photon.clip.effect.draw-cloud-tunnel";
    toReturn.categories.append("Draw");

    return toReturn;
}

} // namespace photon
