#ifndef PHOTON_TIMELINELAYER_H
#define PHOTON_TIMELINELAYER_H

#include "layeritem.h"
#include "photon-global.h"

namespace photon {

class SequenceClip;
class TimelineScene;
class ClipItem;

class TimelineClipLayer : public LayerItem
{
    Q_OBJECT
public:
    TimelineClipLayer(ClipLayer *);
    ~TimelineClipLayer();

    void addClip(SequenceClip *t_clip);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;
    SequenceClip *itemForClip(Clip *) const;

public slots:

    void clipAdded(photon::Clip *);
    void clipRemoved(photon::Clip *);
    void clipModified(photon::Clip *);
    void addRoutine(photon::Routine *t_routine, double t_time);
    void addClip(photon::Clip *t_clip, double t_time);
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

#endif // PHOTON_TIMELINELAYER_H
