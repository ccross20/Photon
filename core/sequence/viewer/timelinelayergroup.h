#ifndef PHOTON_TIMELINELAYERFOLDER_H
#define PHOTON_TIMELINELAYERFOLDER_H

#include "layeritem.h"
#include "photon-global.h"

namespace photon {

class SequenceClip;
class TimelineScene;

class TimelineLayerGroup : public LayerItem
{
    Q_OBJECT
public:
    TimelineLayerGroup(LayerGroup *);
    ~TimelineLayerGroup();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    LayerItem *findLayer(Layer *t_layer);

public slots:

    void layerAdded(photon::Layer *);
    void layerRemoved(photon::Layer *);
    void layerModified(photon::Layer *);
    void removeLayer();

protected:
    void addedToScene(TimelineScene *) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    friend class TimelineScene;
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TIMELINELAYERFOLDER_H
