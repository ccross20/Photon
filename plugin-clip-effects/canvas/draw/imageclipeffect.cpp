#include "imageclipeffect.h"
#include "opengl/openglshader.h"
#include "opengl/openglplane.h"
#include "channel/parameter/fileparameter.h"
#include "channel/parameter/boolchannelparameter.h"

namespace photon {

ImageClipEffect::ImageClipEffect() {

}


void ImageClipEffect::init()
{
    addChannelParameter(new FileParameter("file",""));
    addChannelParameter(new BoolChannelParameter("maintainAspect"));
    /*
    QOffscreenSurface *surface = photonApp->surface();

    QOpenGLContext context;

    context.setShareContext(QOpenGLContext::globalShareContext());
    context.create();
    context.makeCurrent(surface);
    */



}

void ImageClipEffect::initializeContext(QOpenGLContext *t_context, Canvas *)
{
    m_texture = new OpenGLTexture;
    //m_texture->resize(t_context.openglContext, QImage::Format::Format_ARGB32_Premultiplied, 569,619);
    m_plane = new OpenGLPlane(t_context, bounds_d{-1,-1,1,1}, false);
    m_shader = new OpenGLShader(t_context, ":/resources/shader/BasicTextureVertex.vert",
                                ":/resources/shader/texture.frag");
    m_shader->bind(t_context);
}

void ImageClipEffect::canvasResized(QOpenGLContext *, Canvas *)
{

}

void ImageClipEffect::evaluate(CanvasClipEffectEvaluationContext &t_context)
{
    //float w = static_cast<float>(t_context.canvasImage->width());
    //float h = static_cast<float>(t_context.canvasImage->height());

    QString p = t_context.channelValues["file"].toString();
    bool maintainAspect = t_context.channelValues["maintainAspect"].toBool();



    if(path != p && p.length() > 0)
    {
        QImage image{p};

        if(!image.isNull() && m_texture)
        {
            m_texture->loadRaster(t_context.openglContext, &image);


        }
        path = p;
    }


    if(maintainAspect)
    {
        double ratio = static_cast<double>(m_texture->width())/static_cast<double>(m_texture->height());

        double w = (2.0 * ratio)/2.0;

        m_plane->bind(t_context.openglContext);
        m_plane->setBounds(bounds_d{-w,-1,w,1});
    }
    else
    {
        m_plane->bind(t_context.openglContext);
        m_plane->setBounds(bounds_d{-1,-1,1,1});
    }


    m_shader->bind(t_context.openglContext);
    m_shader->setTexture("tex", m_texture->handle());

    m_plane->draw();

}

ClipEffectInformation ImageClipEffect::info()
{
    ClipEffectInformation toReturn([](){return new ImageClipEffect;});
    toReturn.name = "Load Image";
    toReturn.id = "photon.clip.effect.draw-image";
    toReturn.categories.append("Draw");

    return toReturn;
}


} // namespace photon
