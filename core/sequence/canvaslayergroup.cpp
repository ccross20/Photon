#include "canvaslayergroup.h"

namespace photon {

class CanvasLayerGroup::Impl
{
public:
    Canvas *canvas;
};

CanvasLayerGroup::CanvasLayerGroup(Canvas *t_canvas, const QString &t_name):LayerGroup(t_name,"canvasGroup"),m_impl(new Impl)
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

} // namespace photon
