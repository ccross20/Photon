#ifndef PHOTON_TIMELINEMASTERLAYER_H
#define PHOTON_TIMELINEMASTERLAYER_H

#include "layeritem.h"

namespace photon {

class SequenceClip;
class TimelineScene;

class TimelineMasterLayer : public LayerItem
{
    Q_OBJECT
public:
    TimelineMasterLayer(MasterLayer *);
    ~TimelineMasterLayer();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;

public slots:
\
    void removeLayer();

protected:
    void addedToScene(TimelineScene *) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    friend class TimelineScene;
    friend class TimelineLayerGroup;
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TIMELINEMASTERLAYER_H
