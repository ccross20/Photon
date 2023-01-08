#include "layeritem.h"

namespace photon {

class LayerItem::Impl
{
public:
    Layer *layer;
};

LayerItem::LayerItem(Layer *t_layer) : QGraphicsObject(),m_impl(new Impl)
{
    m_impl->layer = t_layer;
}

LayerItem::~LayerItem()
{
    delete m_impl;
}

Layer *LayerItem::layer() const
{
    return m_impl->layer;
}

void LayerItem::addedToScene(TimelineScene *)
{

}

} // namespace photon
