#ifndef PHOTON_TIMELINESCENE_H
#define PHOTON_TIMELINESCENE_H

#include <QGraphicsScene>
#include "photon-global.h"

namespace photon {

class SequenceClip;

class PHOTONCORE_EXPORT TimelineScene : public QGraphicsScene
{
    Q_OBJECT
public:
    TimelineScene(Sequence *sequence = nullptr);
    ~TimelineScene();
    void setSequence(Sequence *);
    Sequence *sequence() const;

private slots:
    void layerAdded(photon::Layer*);
    void layerRemoved(photon::Layer*);
    void createLayer();
    void createCanvasLayerGroup();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TIMELINESCENE_H
