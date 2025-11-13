#include <QMatrix4x4>
#include "canvasaction.h"
#include "opengl/openglresources.h"
#include "photoncore.h"
#include "opengl/openglframebuffer.h"
#include "pixel/canvas.h"
#include "processcontext.h"
#include "pixel/pixellayout.h"
#include "project/project.h"
#include "pixel/pixellayoutcollection.h"
#include "pixel/canvascollection.h"
#include "sequence/canvaseffect.h"
#include "opengl/openglshader.h"
#include "opengl/openglplane.h"
#include "opengl/opengltexture.h"
#include "opengl/openglframebuffer.h"
#include "channel/parameter/numberchannelparameter.h"
#include "channel/parameter/point2channelparameter.h"
#include "surface/viewer/canvasactionsettingswidget.h"

namespace photon {

CanvasAction::CanvasAction(QObject *t_parent) : SurfaceAction(t_parent){

    setId("canvasaction");
    setType("canvas");
    QOffscreenSurface *surface = photonApp->surface();

    m_context = new QOpenGLContext();

    m_context->setShareContext(QOpenGLContext::globalShareContext());
    m_context->create();
    m_context->makeCurrent(surface);


    addChannelParameter(new Point2ChannelParameter("position",QPointF{0,0}));
    addChannelParameter(new Point2ChannelParameter("center",QPointF{0,0}));
    addChannelParameter(new Point2ChannelParameter("scale",QPointF{1,1}));
    addChannelParameter(new NumberChannelParameter("rotation"));


}

CanvasAction::~CanvasAction()
{

}

void CanvasAction::setCanvas(Canvas *t_canvas)
{
    if(m_canvas)
        disconnect(m_canvas, &Canvas::sizeUpdated,this, &CanvasAction::canvasSizeUpdated);
    m_canvas = t_canvas;
    if(t_canvas)
    {
        qDebug() << "Set canvas";
        connect(t_canvas, &Canvas::sizeUpdated,this, &CanvasAction::canvasSizeUpdated);

        m_context->makeCurrent(photonApp->surface());


        m_plane = new OpenGLPlane(m_context, bounds_d{-1,-1,1,1}, false);
        m_shader = new OpenGLShader(m_context, ":/resources/shader/projectedvertex.vert",
                                          ":/resources/shader/TextureOpacity.frag");
        m_shader->bind(m_context);
        m_viewportLoc = m_shader->uniformLocation("projMatrix");
        m_cameraLoc = m_shader->uniformLocation("mvMatrix");

        QMatrix4x4 mat;
        mat.setToIdentity();
        mat.ortho(-1,1, -1,1,-1,1);

        m_shader->setMatrix(m_viewportLoc,mat);
        int w = t_canvas->width();
        int h = t_canvas->height();
        m_texture = new OpenGLTexture;
        m_texture->resize(m_context, QImage::Format::Format_ARGB32_Premultiplied, w, h);



    }

}

QWidget *CanvasAction::settingsWidget() const
{
    return new CanvasActionSettingsWidget(const_cast<CanvasAction*>(this));
}

Canvas *CanvasAction::canvas() const
{
    return m_canvas;
}

QOpenGLContext *CanvasAction::openGLContext() const
{
    return m_context;
}

void CanvasAction::addPixelLayout(PixelLayout *t_layout)
{
    if(m_pixelLayouts.contains(t_layout))
        return;
    m_pixelLayouts << t_layout;
    emit pixelLayoutAdded(t_layout);
}

void CanvasAction::removePixelLayout(PixelLayout *t_layout)
{
    if(m_pixelLayouts.removeOne(t_layout))
        emit pixelLayoutRemoved(t_layout);
}

const QVector<PixelLayout*> &CanvasAction::pixelLayouts() const
{
    return m_pixelLayouts;
}

PixelLayout *CanvasAction::pixelLayoutAtIndex(int t_index) const
{
    return m_pixelLayouts[t_index];
}

int CanvasAction::pixelLayoutCount() const
{
    return m_pixelLayouts.length();
}

void CanvasAction::canvasSizeUpdated(QSize t_size)
{
    m_context->makeCurrent(photonApp->surface());

    for(auto clipEffect : clipEffects())
    {
        auto canvasEffect = dynamic_cast<CanvasEffect*>(clipEffect);
        if(canvasEffect)
            canvasEffect->canvasResized(m_context, m_canvas);
    }

    m_texture->resize(m_context, QImage::Format::Format_ARGB32_Premultiplied, m_canvas->width(), m_canvas->height());
}

void CanvasAction::processChannels(ProcessContext &t_context)
{
    if(!m_canvas)
    {
        return;
    }

    if(!m_canvas->texture())
    {
        qDebug() << "No texture";
        return;
    }


    t_context.canvas = m_canvas;
    m_context->makeCurrent(photonApp->surface());
    t_context.openglContext = m_context;
    OpenGLFrameBuffer buffer(m_canvas->texture(), m_context);
    t_context.frameBuffer = &buffer;
    t_context.resources = photonApp->openGLResources();
    t_context.resources->bind(t_context.openglContext);

    auto f = m_context->functions();

    f->glViewport(0,0,m_canvas->width(), m_canvas->height());
    f->glClearColor(0.0f,0.0f,0.0f,0.0f);
    f->glClear(GL_COLOR_BUFFER_BIT);
    f->glEnable(GL_BLEND);
    f->glBlendEquation (GL_FUNC_ADD);
    //f->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    f->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);



    double amount = strengthAtTime(t_context.relativeTime);

    QPointF pos = t_context.channelValues["position"].value<QPointF>();
    QPointF center = t_context.channelValues["center"].value<QPointF>();
    QPointF scale = t_context.channelValues["scale"].value<QPointF>();
    double rotation = t_context.channelValues["rotation"].toDouble();


    OpenGLFrameBuffer *previousBuffer = t_context.frameBuffer;
    f->glClearColor(.0f,.0f,.0f,.0f);
    f->glClear(GL_COLOR_BUFFER_BIT);
    t_context.frameBuffer = &buffer;

    SurfaceAction::processChannels(t_context);


    t_context.frameBuffer = previousBuffer;
    t_context.frameBuffer->bind();
    m_shader->bind(t_context.openglContext);
    m_texture->bind(t_context.openglContext);
    m_shader->setTexture("tex",m_texture->handle());
    m_shader->setFloat("opacity",  amount);


    QMatrix4x4 camMatrix;
    camMatrix.setToIdentity();
    camMatrix.translate(pos.x()*2.0, pos.y()*2.0);
    camMatrix.translate(center.x(), center.y());
    camMatrix.rotate(rotation,0,0,1);
    camMatrix.scale(scale.x(), scale.y());
    camMatrix.translate(-center.x(), -center.y());
    m_shader->setMatrix(m_cameraLoc, camMatrix);


    m_plane->draw();


    if(!m_pixelLayouts.isEmpty())
    {
        QImage image(m_canvas->width(), m_canvas->height(),QImage::Format_ARGB32_Premultiplied);
        buffer.writeToRaster(&image);

        t_context.image = &image;

        for(auto pixelLayout : m_pixelLayouts)
            pixelLayout->process(t_context, amount);
    }

    buffer.destroy();
    m_canvas->updateTexture();
    m_context->doneCurrent();
}

void CanvasAction::restore(Project &t_project)
{
    if(m_canvasIndex >= 0)
        setCanvas(t_project.canvases()->canvasAtIndex(m_canvasIndex));
    SurfaceAction::restore(t_project);
}

void CanvasAction::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SurfaceAction::readFromJson(t_json, t_context);

    //m_canvasIndex = t_json.value("canvasIndex").toInt(-1);
    m_canvasIndex = 0;

    if(t_json.contains("pixelLayouts"))
    {
        auto pixelLayoutArray = t_json.value("pixelLayouts").toArray();
        for(auto layoutObj : pixelLayoutArray)
        {
            auto layout = t_context.project->pixelLayouts()->findLayoutWithId(layoutObj.toString().toLatin1());

            if(layout)
                m_pixelLayouts.append(layout);
        }
    }
}

void CanvasAction::writeToJson(QJsonObject &t_json) const
{
    SurfaceAction::writeToJson(t_json);

    if(m_canvas)
        t_json.insert("canvasIndex", 0);
    else
        t_json.insert("canvasIndex", -1);

    QJsonArray pixelLayoutArray;
    for(auto pl : m_pixelLayouts)
    {
        pixelLayoutArray.append(QString{pl->uniqueId()});
    }

    t_json.insert("pixelLayouts", pixelLayoutArray);
}


} // namespace photon
