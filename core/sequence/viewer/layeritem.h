#ifndef PHOTON_LAYERITEM_H
#define PHOTON_LAYERITEM_H

#include <QGraphicsObject>
#include "photon-global.h"

namespace photon {

class TimelineScene;

class LayerItem : public QGraphicsObject
{
public:
    LayerItem(Layer *);
    virtual ~LayerItem();

    Layer *layer() const;


protected:
    virtual void addedToScene(TimelineScene *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_LAYERITEM_H
