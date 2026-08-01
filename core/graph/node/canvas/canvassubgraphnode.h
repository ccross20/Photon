#ifndef PHOTON_CANVASSUBGRAPHNODE_H
#define PHOTON_CANVASSUBGRAPHNODE_H

#include <atomic>
#include <QSize>
#include <QVector>
#include <QColor>
#include <QPointF>
#include <QMutex>
#include "model/subgraphnode.h"
#include "photon-global.h"
#include "model/parameter/booleanparameter.h"
#include "model/parameter/integerparameter.h"

class QRhi;
class QRhiTexture;
class QRhiTextureRenderTarget;
class QRhiRenderPassDescriptor;

namespace photon {

class GraphContextNode;
class CanvasOutputNode;
class CanvasRenderManager;
class CanvasDmxSampler;
class ColorParameter;
struct RoutineEvaluationContext;

// A canvas rendered entirely on the GPU (QRhi), embeddable in any host graph
// (bus/surface). Owns an inner graph (graphTypeId "canvas") with an auto-created
// Globals node (resolution/time outputs) and a terminal Output node (the sink
// input). Each evaluation opens an offscreen QRhi frame on the shared RhiContext,
// clears an owned sink texture to the background colour, evaluates the inner
// graph (whose nodes render into pooled textures), then composites the Output
// node's texture into the sink. The sink texture is what preview/DMX read.
//
// Modeled on PixelGraph; see [[canvas-gpu-pipeline]].
class PHOTONCORE_EXPORT CanvasSubGraphNode : public keira::SubGraphNode
{
public:
    const static QByteArray Width;
    const static QByteArray Height;
    const static QByteArray Enabled;
    const static QByteArray Background;
    const static QByteArray CanvasSubGraphId;

    CanvasSubGraphNode();
    ~CanvasSubGraphNode();

    void createParameters() override;

    // Runs on the graph's worker thread: snapshots inputs and flags a render.
    // Does NOT touch the GPU (QRhi is main-thread only). With no CanvasRenderManager
    // (headless tests), renders inline instead.
    void evaluate(keira::EvaluationContext *) const override;

    // Runs on the MAIN thread (called by CanvasRenderManager): evaluates the inner
    // graph and does all QRhi work — open frame, clear sink, composite output.
    void renderMainThread() const;

    // Consumes the dirty flag set by the last evaluate(). Returns true if a render
    // is due. Called by the manager on the main thread.
    bool takeNeedsRender() { return m_needsRender.exchange(false); }

    static keira::NodeInformation info();

    void readFromJson(const QJsonObject &, keira::NodeLibrary *library) override;

    // The canvas sink texture (RGBA8, RenderTarget | UsedAsTransferSource), or
    // null before the first render. Read by preview/DMX. Main-thread only.
    QRhiTexture *outputTexture() const;
    QSize canvasSize() const;

    // The CanvasOutputNodes in the inner graph (auto "Output" + any user-added).
    QVector<CanvasOutputNode *> outputNodes() const;

    // The canvas background colour (what transparent regions flatten onto).
    QColor backgroundColor() const;

protected:
    keira::NodeLibrary *nodeLibrary() const override;

private:
    // (Re)creates the sink texture + render target when missing or resized. Runs
    // inside evaluate(), on the shared device. Returns false if creation failed.
    bool ensureSink(QRhi *rhi, const QSize &size) const;
    void releaseSink() const;

    // Worker-thread: each Output node writes the colours it gathered into the
    // frame's DMX matrix via its assigned layouts.
    void sampleDmx(RoutineEvaluationContext *context) const;

    keira::IntegerParameter *m_widthParam = nullptr;
    keira::IntegerParameter *m_heightParam = nullptr;
    keira::BooleanParameter *m_enabledParam = nullptr;
    ColorParameter *m_backgroundParam = nullptr;

    GraphContextNode *m_globalsNode = nullptr;
    CanvasOutputNode *m_outputNode = nullptr;

    // Non-null in the app (registered for main-thread rendering); null in headless
    // tests, where evaluate() renders inline.
    CanvasRenderManager *m_manager = nullptr;

    // Set by evaluate() (worker thread), consumed by the manager (main thread).
    mutable std::atomic<bool> m_needsRender{false};

    // Snapshotted by evaluate() for renderMainThread() to consume. Cross-thread
    // reads of plain doubles are benign here (last-writer-wins per frame).
    mutable double m_relativeTime = 0.0;
    mutable double m_globalTime = 0.0;
    mutable RhiContext *m_rhiContext = nullptr;   // set only in headless tests via the context

    // Owned sink resources. Mutable because render is const (keira contract) but
    // must lazily build/resize GPU resources. Main-thread only.
    mutable QRhiTexture *m_canvasTexture = nullptr;
    mutable QRhiTextureRenderTarget *m_canvasRT = nullptr;
    mutable QRhiRenderPassDescriptor *m_canvasRP = nullptr;
    mutable QSize m_canvasSize;

};

} // namespace photon

#endif // PHOTON_CANVASSUBGRAPHNODE_H
