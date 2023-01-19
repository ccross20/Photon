#include <QImage>
#include "canvaslayergroup.h"
#include "pixel/canvas.h"
#include "sequence.h"
#include "project/project.h"
#include "pixel/canvascollection.h"

namespace photon {

class CanvasLayerGroup::Impl
{
public:
    Canvas *canvas;
    int canvasIndex = -1;
};

CanvasLayerGroup::CanvasLayerGroup(QObject *t_parent):LayerGroup("CanvasGroup", t_parent),m_impl(new Impl)
{
    m_impl->canvas = nullptr;
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
    QImage image(m_impl->canvas->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::black);
    t_context.canvasImage = &image;
    LayerGroup::processChannels(t_context);

    m_impl->canvas->paint(image);
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
