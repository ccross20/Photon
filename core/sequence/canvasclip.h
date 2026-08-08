#ifndef PHOTON_CANVASCLIP_H
#define PHOTON_CANVASCLIP_H
#include "clip.h"


namespace photon {

class GraphContextNode;
class CanvasOutputNode;

// A clip whose content is a private embedded canvas graph (graphTypeId
// "canvas", auto-created Globals + Output nodes - same shape as
// CanvasSubGraphNode's inner graph, see canvassubgraphnode.cpp). The clip's
// position/center/scale/rotation channels plus its built-in Strength
// channel drive how that graph's rendered output is composited onto the
// owning CanvasLayerGroup's sink - see CanvasLayerGroup::renderMainThread().
class PHOTONCORE_EXPORT CanvasClip : public Clip
{
    Q_OBJECT
public:
    CanvasClip();
    CanvasClip(double start, double duration, QObject *parent = nullptr);
    ~CanvasClip();

    keira::Graph *contentGraph() const override;

    void processChannels(ProcessContext &) override;

    void restore(Project &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

    static ClipInformation info();

protected:
    // Caches the owning CanvasLayerGroup (if any) whenever this clip is
    // attached to a layer, so it's still known at destruction time even
    // though layer()/parentGroup() are already cleared by then - see
    // ~CanvasClip().
    void layerDidChange(Layer *) override;

private:
    class Impl;
    Impl *m_impl;
};


} // namespace photon

#endif // PHOTON_CANVASCLIP_H
