#include <QImage>
#include <QOffscreenSurface>
#include "canvaslayergroup.h"
#include "pixel/canvas.h"
#include "sequence.h"
#include "project/project.h"
#include "pixel/canvascollection.h"
#include "photoncore.h"
#include "opengl/openglframebuffer.h"

namespace photon {

class CanvasLayerGroup::Impl
{
public:
    Canvas *canvas;
    QOpenGLContext *context = nullptr;
    QImage tempImage;
    int canvasIndex = -1;
};

CanvasLayerGroup::CanvasLayerGroup(QObject *t_parent):LayerGroup("CanvasGroup", t_parent),m_impl(new Impl)
{
    m_impl->canvas = nullptr;

    QOffscreenSurface *surface = photonApp->surface();

    m_impl->context = new QOpenGLContext();

    m_impl->context->setShareContext(QOpenGLContext::globalShareContext());
    m_impl->context->create();
    m_impl->context->makeCurrent(surface);
}

CanvasLayerGroup::CanvasLayerGroup(Canvas *t_canvas, const QString &t_name):LayerGroup(t_name,"CanvasGroup"),m_impl(new Impl)
{
    m_impl->canvas = t_canvas;
}

CanvasLayerGroup::~CanvasLayerGroup()
{
    delete m_impl;
}

void CanvasLayerGroup::setCanvas(Canvas *t_canvas)
{
    m_impl->canvas = t_canvas;
}

Canvas *CanvasLayerGroup::canvas() const
{
    return m_impl->canvas;
}

void CanvasLayerGroup::processChannels(ProcessContext &t_context)
{
    if(!m_impl->canvas->texture())
    {
        qDebug() << "No texture";
        return;
    }


    t_context.canvas = m_impl->canvas;
    m_impl->context->makeCurrent(photonApp->surface());
    t_context.openglContext = m_impl->context;
    OpenGLFrameBuffer buffer(m_impl->canvas->texture(), m_impl->context);
    t_context.frameBuffer = &buffer;

    auto f = m_impl->context->functions();

    f->glViewport(0,0,m_impl->canvas->width(), m_impl->canvas->height());
    f->glClearColor(0.0f,0.0f,0.0f,1.0f);
    f->glClear(GL_COLOR_BUFFER_BIT);


    /*
    QImage image(m_impl->canvas->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::black);
    t_context.previousCanvasImage = &m_impl->tempImage;
    t_context.canvasImage = &image;
*/
    LayerGroup::processChannels(t_context);

    //context.doneCurrent();


    buffer.destroy();

    m_impl->canvas->updateTexture();


    m_impl->context->doneCurrent();
}

void CanvasLayerGroup::restore(Project &t_project)
{
    if(m_impl->canvasIndex >= 0)
        m_impl->canvas = t_project.canvases()->canvasAtIndex(m_impl->canvasIndex);
}

void CanvasLayerGroup::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    LayerGroup::readFromJson(t_json, t_context);

    m_impl->canvasIndex = t_json.value("canvasIndex").toInt(-1);
}

void CanvasLayerGroup::writeToJson(QJsonObject &t_json) const
{
    LayerGroup::writeToJson(t_json);

    if(m_impl->canvas)
        t_json.insert("canvasIndex", 0);
    else
        t_json.insert("canvasIndex", -1);
}

} // namespace photon
