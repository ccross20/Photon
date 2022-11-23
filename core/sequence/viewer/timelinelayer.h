#ifndef PHOTON_TIMELINELAYER_H
#define PHOTON_TIMELINELAYER_H

#include <QGraphicsObject>
#include "photon-global.h"

namespace photon {

class SequenceClip;
class TimelineScene;

class TimelineLayer : public QGraphicsObject
{
    Q_OBJECT
public:
    TimelineLayer(Layer *);
    ~TimelineLayer();

    void addClip(SequenceClip *t_clip);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;

    Layer *layer() const;

public slots:

    void clipAdded(photon::Clip *);
    void clipRemoved(photon::Clip *);
    void clipModified(photon::Clip *);
    void addRoutine(photon::Routine *t_routine, double t_time);
    void removeLayer();

protected:
    void addedToScene(TimelineScene *);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    friend class TimelineScene;
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TIMELINELAYER_H
