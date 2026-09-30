#ifndef PHOTON_TIMELINESCENE_H
#define PHOTON_TIMELINESCENE_H

#include <QGraphicsScene>
#include "photon-global.h"

namespace photon {

class SequenceClip;
class LayerItem;

class PHOTONCORE_EXPORT TimelineScene : public QGraphicsScene
{
    Q_OBJECT
public:
    TimelineScene(Sequence *sequence = nullptr);
    ~TimelineScene();
    void setSequence(Sequence *);
    Sequence *sequence() const;
    LayerItem *layerAtY(double) const;
    LayerItem *itemForLayer(Layer*) const;
    SequenceClip *itemForClip(Clip*) const;

    // The layer new clips are pasted into. Editor state only - not saved.
    // Defaults to the first clip layer.
    ClipLayer *activeLayer() const;
    void setActiveLayer(photon::ClipLayer *);

    QVector<Clip*> selectedClips() const;
    // Clipboard (see ClipClipboard). Paste goes into `layer`, or the active
    // layer when null, and leaves the pasted clips selected.
    void copySelectedClips();
    void cutSelectedClips();
    void pasteClips(double time, photon::ClipLayer *layer = nullptr);

signals:
    void activeLayerChanged(photon::ClipLayer *);

private slots:
    void layerAdded(photon::Layer*);
    void layerRemoved(photon::Layer*);
    void createLayer();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TIMELINESCENE_H
