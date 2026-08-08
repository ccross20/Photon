#ifndef PHOTON_CANVASLAYERGROUP_H
#define PHOTON_CANVASLAYERGROUP_H
#include <atomic>
#include <QWidget>
#include <QColor>
#include <QPointF>
#include "layergroup.h"
#include "graph/node/canvas/canvasrenderable.h"

class QRhi;
class QRhiTexture;
class QRhiTextureRenderTarget;
class QRhiRenderPassDescriptor;

namespace photon {

class CanvasClip;

// A snapshot of one active CanvasClip's transform for this frame, queued by
// CanvasClip::processChannels() (worker or GUI thread) and consumed by
// CanvasLayerGroup::renderMainThread() (main thread) to composite that
// clip's rendered content onto the group's sink.
struct CanvasClipRenderState
{
    CanvasClip *clip = nullptr;
    QPointF position{0.0, 0.0};
    QPointF center{0.5, 0.5};
    QPointF scale{1.0, 1.0};
    double rotation = 0.0;
    double strength = 1.0;
    double relativeTime = 0.0;
    double globalTime = 0.0;
};

class PHOTONCORE_EXPORT CanvasLayerGroupEditor : public QWidget
{
    Q_OBJECT
public:
    CanvasLayerGroupEditor(CanvasLayerGroup *);

public slots:
    void openAddPixelLayout();
    void removeSelectedLayout();
    void pixelLayoutAdded(photon::PixelLayout *);
    void pixelLayoutRemoved(photon::PixelLayout *);
    void widthChanged(int);
    void heightChanged(int);
    void backgroundChanged(QColor);

private:
    class Impl;
    Impl *m_impl;
};

// A sequence layer group that owns and renders its own GPU (QRhi) canvas -
// the same sink-texture-ownership pattern as CanvasSubGraphNode
// (core/graph/node/canvas/canvassubgraphnode.h), rather than referencing an
// externally shared pixel::Canvas by index into a CanvasCollection (the old
// model). Registers with CanvasRenderManager for main-thread rendering, since
// processChannels() can run on either the GUI thread or keira's eval thread
// (via a SequenceNode) and QRhi is main-thread only.
class PHOTONCORE_EXPORT CanvasLayerGroup : public LayerGroup, public CanvasRenderable
{
    Q_OBJECT
public:
    explicit CanvasLayerGroup(QObject *parent = nullptr);
    CanvasLayerGroup(const QString &name, QObject *parent = nullptr);
    ~CanvasLayerGroup();

    // Pixel dimensions of the canvas itself. Named to avoid colliding with
    // the inherited virtual Layer::height(), which the timeline UI uses for
    // this layer's row height - a name clash there previously made the
    // group's timeline header take on the canvas's pixel height (e.g. 256px)
    // instead of its actual computed row height.
    int canvasWidth() const;
    int canvasHeight() const;
    void setCanvasWidth(int);
    void setCanvasHeight(int);
    QColor background() const;
    void setBackground(const QColor &);

    void addPixelLayout(PixelLayout *);
    void removePixelLayout(PixelLayout *);
    PixelLayout *pixelLayoutAtIndex(int index) const;
    int pixelLayoutCount() const;
    const QVector<PixelLayout*> &pixelLayouts() const;

    QVector<PixelSource*> sources() const;

    // The canvas sink texture, or null before the first render. Main-thread only.
    QRhiTexture *outputTexture() const;
    QSize canvasSize() const;

    QWidget *createEditor() override;
    void processChannels(ProcessContext &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

    // Called by an active CanvasClip's processChannels() (worker or GUI
    // thread) to report its resolved transform for this frame. Thread-safe.
    void queueClipForRender(const CanvasClipRenderState &);

    // Called by ~CanvasClip() before it tears down its graph. Blocks until
    // any in-progress renderMainThread() (which holds the same lock across
    // its whole clip-compositing loop) is done with this clip, then purges
    // any queued/active reference to it. Safe to call even if this group is
    // itself mid-teardown (see m_tornDown below) - a no-op in that case,
    // since this group's own destructor already guarantees no render can be
    // in flight by the time it starts destroying child clips.
    void clipBeingDestroyed(CanvasClip *);

    // CanvasRenderable
    bool takeNeedsRender() const override;
    void renderMainThread() const override;

signals:

    void pixelLayoutAdded(photon::PixelLayout *);
    void pixelLayoutRemoved(photon::PixelLayout *);

private:
    bool ensureSink(QRhi *rhi, const QSize &size) const;
    void releaseSink() const;

    // Set as the very first statement of ~CanvasLayerGroup(), before m_impl
    // is touched - a plain member (not in Impl) so it stays valid for the
    // rest of the destructor chain, including the QObject cascade that then
    // destroys child ClipLayers/CanvasClips (whose destructors may otherwise
    // call back into this group via clipBeingDestroyed()).
    std::atomic<bool> m_tornDown{false};

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CANVASLAYERGROUP_H
