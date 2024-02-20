#include <QMatrix4x4>
#include "canvasclip.h"
#include "photoncore.h"
#include "project/project.h"
#include "opengl/openglshader.h"
#include "opengl/openglplane.h"
#include "opengl/opengltexture.h"
#include "opengl/openglframebuffer.h"
#include "pixel/canvas.h"
#include "sequence.h"
#include "channel/parameter/numberchannelparameter.h"
#include "channel/parameter/point2channelparameter.h"
#include "clipeffect.h"
#include "canvasclipeffect.h"
#include "canvaslayergroup.h"

namespace photon {

class CanvasClip::Impl
{
public:
    OpenGLShader *shader = nullptr;
    OpenGLPlane *plane = nullptr;
    OpenGLTexture *texture = nullptr;
    int viewportLoc;
    int cameraLoc;
};

CanvasClip::CanvasClip(): Clip(),m_impl(new Impl)
{
    setId("canvasclip");
    addChannelParameter(new Point2ChannelParameter("position",QPointF{0,0}));
    addChannelParameter(new Point2ChannelParameter("center",QPointF{0,0}));
    addChannelParameter(new Point2ChannelParameter("scale",QPointF{1,1}));
    addChannelParameter(new NumberChannelParameter("rotation"));
}

CanvasClip::CanvasClip(double t_start, double t_duration, QObject *t_parent) : Clip(t_start, t_duration, t_parent),m_impl(new Impl)
{    
    setId("canvasclip");
    addChannelParameter(new Point2ChannelParameter("position",QPointF{0,0}));
    addChannelParameter(new Point2ChannelParameter("center",QPointF{0,0}));
    addChannelParameter(new Point2ChannelParameter("scale",QPointF{1,1}));
    addChannelParameter(new NumberChannelParameter("rotation"));
}

CanvasClip::~CanvasClip()
{
    delete m_impl;
}

void CanvasClip::layerDidChange(Layer *t_layer)
{
    auto group = dynamic_cast<CanvasLayerGroup*>(t_layer->parentGroup());
    if(group)
    {
        group->openGLContext()->makeCurrent(photonApp->surface());
        initializeContext(group->openGLContext(), group->canvas());
    }
    Clip::layerDidChange(t_layer);
}

void CanvasClip::initializeContext(QOpenGLContext *t_context, Canvas *t_canvas)
{
    if(!t_canvas)
        return;

    for(auto clipEffect : clipEffects())
    {
        auto canvasEffect = dynamic_cast<CanvasClipEffect*>(clipEffect);
        if(canvasEffect)
            canvasEffect->initializeContext(t_context, t_canvas);
    }

    m_impl->plane = new OpenGLPlane(t_context, bounds_d{-1,-1,1,1}, false);
    m_impl->shader = new OpenGLShader(t_context, ":/resources/shader/projectedvertex.vert",
                                      ":/resources/shader/TextureOpacity.frag");
    m_impl->shader->bind(t_context);
    m_impl->viewportLoc = m_impl->shader->uniformLocation("projMatrix");
    m_impl->cameraLoc = m_impl->shader->uniformLocation("mvMatrix");

    QMatrix4x4 mat;
    mat.setToIdentity();
    mat.ortho(-1,1, -1,1,-1,1);

    m_impl->shader->setMatrix(m_impl->viewportLoc,mat);
    int w = t_canvas->width();
    int h = t_canvas->height();
    m_impl->texture = new OpenGLTexture;
    m_impl->texture->resize(t_context, QImage::Format::Format_ARGB32_Premultiplied, w, h);

}

void CanvasClip::canvasResized(QOpenGLContext *t_context, Canvas *t_canvas)
{
    for(auto clipEffect : clipEffects())
    {
        auto canvasEffect = dynamic_cast<CanvasClipEffect*>(clipEffect);
        if(canvasEffect)
            canvasEffect->canvasResized(t_context, t_canvas);
    }

    m_impl->texture->resize(t_context, QImage::Format::Format_ARGB32_Premultiplied, t_canvas->width(), t_canvas->height());
}

void CanvasClip::processChannels(ProcessContext &t_context)
{

    double amount = strengthAtTime(t_context.relativeTime);

    QPointF pos = t_context.channelValues["position"].value<QPointF>();
    QPointF center = t_context.channelValues["center"].value<QPointF>();
    QPointF scale = t_context.channelValues["scale"].value<QPointF>();
    double rotation = t_context.channelValues["rotation"].toDouble();


    auto f = t_context.openglContext->functions();

    OpenGLFrameBuffer *previousBuffer = t_context.frameBuffer;
    OpenGLFrameBuffer buffer(m_impl->texture, t_context.openglContext);
    f->glClearColor(.0f,.0f,.0f,.0f);
    f->glClear(GL_COLOR_BUFFER_BIT);
    t_context.frameBuffer = &buffer;

    Clip::processChannels(t_context);

    t_context.frameBuffer = previousBuffer;
    t_context.frameBuffer->bind();
    m_impl->shader->bind(t_context.openglContext);
    m_impl->texture->bind(t_context.openglContext);
    m_impl->shader->setTexture("tex",m_impl->texture->handle());
    m_impl->shader->setFloat("opacity",  amount);


    QMatrix4x4 camMatrix;
    camMatrix.setToIdentity();
    camMatrix.translate(pos.x()*2.0, pos.y()*2.0);
    camMatrix.translate(center.x(), center.y());
    camMatrix.rotate(rotation,0,0,1);
    camMatrix.scale(scale.x(), scale.y());
    camMatrix.translate(-center.x(), -center.y());
    m_impl->shader->setMatrix(m_impl->cameraLoc, camMatrix);


    m_impl->plane->draw();


}


void CanvasClip::restore(Project &t_project)
{
    Clip::restore(t_project);
}

void CanvasClip::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    Clip::readFromJson(t_json, t_context);

}

void CanvasClip::writeToJson(QJsonObject &t_json) const
{
    Clip::writeToJson(t_json);

}


ClipInformation CanvasClip::info()
{
    ClipInformation toReturn([](){return new CanvasClip;});
    toReturn.name = "CanvasClip";
    toReturn.id = "canvasclip";
    //toReturn.categories.append("Generator");

    return toReturn;
}

} // namespace photon
