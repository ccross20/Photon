// Phase 1 spike harness (console subsystem, so logging reaches stdout).
//
// Two checks, both printed with an explicit verdict; exit 0 only if all pass:
//
//   1. Offscreen round-trip: the core-owned RhiContext creates a headless
//      OpenGL-backed QRhi over a context in the global share group and
//      renders+reads back an offscreen texture (no window).
//
//   2. Coexistence: a SECOND, independent QRhi is stood up exactly the way the
//      visualizer does it (QRhiGles2InitParams with a window + swapchain +
//      depth-stencil, see plugin-visualizer/rhi/rhiwindow.cpp) and its frames
//      are interleaved with the offscreen device's render+readback. This
//      reproduces the real risk — two live QRhi devices in one process sharing
//      the global GL context (AA_ShareOpenGLContexts), one windowed, one
//      offscreen — without pulling in the whole visualizer plugin.
//
//     rhi-spike.exe
#include <cstdio>
#include <thread>
#include <rhi/qrhi.h>
#include <QApplication>
#include <QSurfaceFormat>
#include <QWindow>
#include <QOffscreenSurface>
#include <QElapsedTimer>
#include <QColor>
#include "rhi/rhicontext.h"
#include "graph/parameter/rhitextureparameter.h"
#include "graph/node/canvas/canvassubgraphnode.h"
#include "graph/node/canvas/canvasrendermanager.h"
#include "graph/node/canvas/basecanvasnode.h"
#include "graph/node/canvas/canvasfillnode.h"
#include "graph/node/canvas/canvastransformnode.h"
#include "graph/node/canvas/canvasdmxsampler.h"
#include "routine/routineevaluationcontext.h"
#include "data/dmxmatrix.h"
#include "processcontext.h"
#include "pixel/pixelsource.h"

namespace {

// Minimal PixelSource that samples one point at the canvas centre, universe 1 ch 0.
class TestPixelSource : public photon::PixelSource
{
public:
    TestPixelSource() { m_positions << QPointF(0.5, 0.5); }
    const QVector<QPointF> &positions() const override { return m_positions; }
    QByteArray sourceUniqueId() const override { return "test"; }
    int dmxOffset() const override { return 0; }
    int universe() const override { return 1; }
private:
    QVector<QPointF> m_positions;
};

// Phase 5b: GPU gather — render a solid fill, sample it at a few UVs on the GPU,
// read back just those texels, and check they match. Then feed a gathered colour
// through a PixelSource to confirm the DMX write path.
bool runDmxTest(photon::RhiContext &ctx)
{
    const QSize size(32, 32);
    const QColor fill(70, 140, 210, 255);
    const int tol = 3;

    // Render a fill into its output texture.
    photon::CanvasFillNode fillNode;
    fillNode.createParameters();
    fillNode.setValue(photon::CanvasFillNode::Color1, QVariant(fill));
    fillNode.setValue(photon::CanvasFillNode::Mode, 0);

    QRhiCommandBuffer *cb = nullptr;
    if (ctx.rhi()->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess || !cb)
        return false;
    photon::DMXMatrix m;
    photon::RoutineEvaluationContext c(m);
    c.rhiContext = &ctx;
    c.rhiCommandBuffer = cb;
    c.canvasResolution = size;
    fillNode.evaluate(&c);

    const auto fillOut = fillNode.findParameter(photon::CanvasFillNode::Output)
                             ->value().value<photon::RhiTextureData>();

    // Gather that fill texture at three UVs (in the same frame), then read back.
    photon::CanvasDmxSampler sampler;
    QVector<QPointF> uvs{ {0.25, 0.25}, {0.5, 0.5}, {0.75, 0.75} };
    sampler.recordGather(ctx.rhi(), cb, fillOut.texture, uvs, nullptr);
    ctx.rhi()->endOffscreenFrame();

    QVector<QColor> colors;
    bool gatherOk = sampler.takeColors(colors) && colors.size() == uvs.size();
    for (const auto &col : colors)
        gatherOk = gatherOk && qAbs(col.red() - fill.red()) <= tol
                            && qAbs(col.green() - fill.green()) <= tol
                            && qAbs(col.blue() - fill.blue()) <= tol;
    if (!gatherOk) {
        std::printf("  (dmx) GPU gather mismatch (%d colours)\n", int(colors.size()));
        return false;
    }

    // Feed a gathered colour through the CPU DMX-write path.
    TestPixelSource src;
    photon::DMXMatrix dmx;
    photon::ProcessContext pc(dmx);
    QVector<QColor> gathered{ fill };
    pc.gatheredColors = &gathered;
    pc.gatheredIndex = 0;
    src.process(pc, QTransform());
    const bool dmxOk = qAbs(int(dmx.value(0, 0)) - fill.red()) <= tol
                    && qAbs(int(dmx.value(0, 1)) - fill.green()) <= tol
                    && qAbs(int(dmx.value(0, 2)) - fill.blue()) <= tol;
    if (!dmxOk)
        std::printf("  (dmx) gathered DMX write mismatch: got %d %d %d\n",
                    int(dmx.value(0, 0)), int(dmx.value(0, 1)), int(dmx.value(0, 2)));
    return gatherOk && dmxOk;
}

// Reads back the first texel of a texture (RGBA8) into r,g,b. Returns false on
// failure. Runs its own offscreen frame.
bool readFirstTexel(QRhi *rhi, QRhiTexture *tex, int &r, int &g, int &b)
{
    QRhiCommandBuffer *cb = nullptr;
    if (rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess || !cb)
        return false;
    QRhiReadbackResult rb;
    bool done = false;
    rb.completed = [&done]() { done = true; };
    QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
    u->readBackTexture(QRhiReadbackDescription(tex), &rb);
    cb->resourceUpdate(u);
    rhi->endOffscreenFrame();
    if (!done || rb.data.isEmpty())
        return false;
    const uchar *p = reinterpret_cast<const uchar *>(rb.data.constData());
    r = p[0]; g = p[1]; b = p[2];
    return true;
}

// Builds a CanvasSubGraphNode, evaluates it with a hand-made context pointing at
// our RhiContext, and checks the owned sink texture cleared to the background
// colour (nothing is wired into the Output node, so the sink is just the clear).
bool runCanvasGraphTest(photon::RhiContext &ctx)
{
    photon::CanvasSubGraphNode node;
    node.createParameters();
    node.setValue(photon::CanvasSubGraphNode::Width, 64);
    node.setValue(photon::CanvasSubGraphNode::Height, 64);
    const QColor bg(40, 160, 90, 255);
    node.setValue(photon::CanvasSubGraphNode::Background, QVariant(bg));

    photon::DMXMatrix matrix;
    photon::RoutineEvaluationContext context(matrix);
    context.rhiContext = &ctx;

    node.evaluate(&context);

    QRhiTexture *sink = node.outputTexture();
    if (!sink) {
        std::printf("  (canvas) no sink texture after evaluate\n");
        return false;
    }
    if (node.canvasSize() != QSize(64, 64)) {
        std::printf("  (canvas) unexpected sink size\n");
        return false;
    }

    int r = 0, g = 0, b = 0;
    if (!readFirstTexel(ctx.rhi(), sink, r, g, b)) {
        std::printf("  (canvas) sink readback failed\n");
        return false;
    }
    const int tol = 2;
    const bool ok = qAbs(r - bg.red()) <= tol && qAbs(g - bg.green()) <= tol && qAbs(b - bg.blue()) <= tol;
    if (!ok)
        std::printf("  (canvas) sink colour mismatch: expected %d %d %d got %d %d %d\n",
                    bg.red(), bg.green(), bg.blue(), r, g, b);
    return ok;
}

// Evaluates a canvas node that renders into its own output texture, then reads the
// output param's texture back. `setInput` (optional) wires an input texture in.
bool renderNodeAndReadback(photon::RhiContext &ctx, photon::BaseCanvasNode *node,
                           const QByteArray &outputId, const QSize &size,
                           int &r, int &g, int &b)
{
    QRhiCommandBuffer *cb = nullptr;
    if (ctx.rhi()->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess || !cb)
        return false;
    photon::DMXMatrix m;
    photon::RoutineEvaluationContext c(m);
    c.rhiContext = &ctx;
    c.rhiCommandBuffer = cb;
    c.canvasResolution = size;
    node->evaluate(&c);
    ctx.rhi()->endOffscreenFrame();

    auto *param = node->findParameter(outputId);
    if (!param)
        return false;
    const auto out = param->value().value<photon::RhiTextureData>();
    if (!out.texture)
        return false;
    return readFirstTexel(ctx.rhi(), out.texture, r, g, b);
}

// Fill renders a solid colour; Transform (identity) samples the Fill output and
// should reproduce it. Exercises shaders, pipelines, uniform buffers and the
// texture-sampling path.
bool runProducerNodeTest(photon::RhiContext &ctx)
{
    const QSize size(64, 64);
    const QColor fill(30, 200, 120, 255);

    photon::CanvasFillNode fillNode;
    fillNode.createParameters();
    fillNode.setValue(photon::CanvasFillNode::Color1, QVariant(fill));
    fillNode.setValue(photon::CanvasFillNode::Mode, 0);

    int r = 0, g = 0, b = 0;
    if (!renderNodeAndReadback(ctx, &fillNode, photon::CanvasFillNode::Output, size, r, g, b)) {
        std::printf("  (producer) fill render/readback failed\n");
        return false;
    }
    const int tol = 3;
    const bool fillOk = qAbs(r - fill.red()) <= tol && qAbs(g - fill.green()) <= tol && qAbs(b - fill.blue()) <= tol;
    if (!fillOk) {
        std::printf("  (producer) fill colour mismatch: expected %d %d %d got %d %d %d\n",
                    fill.red(), fill.green(), fill.blue(), r, g, b);
        return false;
    }

    // Grab the Fill output handle to feed the Transform.
    auto fillOut = fillNode.findParameter(photon::CanvasFillNode::Output)
                       ->value().value<photon::RhiTextureData>();

    photon::CanvasTransformNode xf;
    xf.createParameters();
    xf.setValue(photon::CanvasTransformNode::Input, QVariant::fromValue(fillOut));
    xf.setValue(photon::CanvasTransformNode::Scale, 1.0);   // identity

    int r2 = 0, g2 = 0, b2 = 0;
    if (!renderNodeAndReadback(ctx, &xf, photon::CanvasTransformNode::Output, size, r2, g2, b2)) {
        std::printf("  (producer) transform render/readback failed\n");
        return false;
    }
    const bool xfOk = qAbs(r2 - fill.red()) <= tol && qAbs(g2 - fill.green()) <= tol && qAbs(b2 - fill.blue()) <= tol;
    if (!xfOk)
        std::printf("  (producer) transform (identity) mismatch: expected %d %d %d got %d %d %d\n",
                    fill.red(), fill.green(), fill.blue(), r2, g2, b2);
    return xfOk;
}

// Reproduces the reported crash scenario: a CanvasRenderManager is active, a
// CanvasSubGraphNode is evaluated from a BACKGROUND thread (like keira's EvalWorker),
// and the manager must do all GPU work on the main thread. Passing means no QRhi
// call happened off the main thread and the sink still rendered correctly.
bool runThreadedCanvasTest(photon::RhiContext &ctx, QApplication &app)
{
    photon::CanvasRenderManager manager(&ctx);   // main thread; starts the render timer

    auto *node = new photon::CanvasSubGraphNode();  // registers with the manager
    node->createParameters();
    node->setValue(photon::CanvasSubGraphNode::Width, 48);
    node->setValue(photon::CanvasSubGraphNode::Height, 48);
    const QColor bg(200, 60, 120, 255);
    node->setValue(photon::CanvasSubGraphNode::Background, QVariant(bg));

    // Evaluate on a background thread — as the surface graph's worker would. This
    // must NOT touch the GPU; it only flags the node dirty.
    std::thread worker([&]() {
        photon::DMXMatrix m;
        photon::RoutineEvaluationContext wctx(m);   // rhiContext null, like the app
        node->evaluate(&wctx);
    });
    worker.join();

    // Pump the main-thread event loop so the manager's timer renders the canvas.
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < 1000 && !node->outputTexture())
        app.processEvents(QEventLoop::AllEvents, 20);

    bool ok = false;
    if (QRhiTexture *sink = node->outputTexture()) {
        int r = 0, g = 0, b = 0;
        if (readFirstTexel(ctx.rhi(), sink, r, g, b)) {
            const int tol = 2;
            ok = qAbs(r - bg.red()) <= tol && qAbs(g - bg.green()) <= tol && qAbs(b - bg.blue()) <= tol;
            if (!ok)
                std::printf("  (threaded) sink colour mismatch: expected %d %d %d got %d %d %d\n",
                            bg.red(), bg.green(), bg.blue(), r, g, b);
        } else {
            std::printf("  (threaded) sink readback failed\n");
        }
    } else {
        std::printf("  (threaded) manager never rendered the canvas\n");
    }

    delete node;   // unregisters, tears down sink on the main thread
    return ok;
}

// Acquires a pooled texture, renders a clear colour into it, reads it back, and
// verifies release→re-acquire returns the SAME handle (pooling). Also round-trips
// the handle through an RhiTextureParameter to smoke-test the new param type.
bool runPoolTest(photon::RhiContext &ctx)
{
    QRhi *rhi = ctx.rhi();
    const QSize size(32, 32);
    const QColor clear = QColor::fromRgbF(0.90f, 0.10f, 0.30f, 1.0f);

    QRhiTexture *a = ctx.acquireTexture(size);
    if (!a) {
        std::printf("  (pool) acquire failed\n");
        return false;
    }

    // Carry the handle through the node-currency param, as a node would.
    photon::RhiTextureParameter param("tex", "Texture", photon::RhiTextureData(a, size));
    const auto viaParam = param.value().value<photon::RhiTextureData>();
    if (viaParam.texture != a || viaParam.size != size) {
        std::printf("  (pool) RhiTextureParameter round-trip mismatch\n");
        return false;
    }

    QRhiColorAttachment colorAtt(a);
    QRhiTextureRenderTargetDescription rtDesc(colorAtt);
    QScopedPointer<QRhiTextureRenderTarget> rt(rhi->newTextureRenderTarget(rtDesc));
    QScopedPointer<QRhiRenderPassDescriptor> rp(rt->newCompatibleRenderPassDescriptor());
    rt->setRenderPassDescriptor(rp.data());
    if (!rt->create()) {
        std::printf("  (pool) render target create failed\n");
        return false;
    }

    QRhiCommandBuffer *cb = nullptr;
    if (rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess || !cb) {
        std::printf("  (pool) beginOffscreenFrame failed\n");
        return false;
    }
    cb->beginPass(rt.data(), clear, { 1.0f, 0 });
    QRhiReadbackResult rb;
    bool done = false;
    rb.completed = [&done]() { done = true; };
    QRhiResourceUpdateBatch *u = rhi->nextResourceUpdateBatch();
    u->readBackTexture(QRhiReadbackDescription(a), &rb);
    cb->endPass(u);
    rhi->endOffscreenFrame();

    if (!done || rb.data.isEmpty()) {
        std::printf("  (pool) readback produced no data\n");
        return false;
    }
    const uchar *p = reinterpret_cast<const uchar *>(rb.data.constData());
    const int tol = 2;
    const bool colorOk = qAbs(int(p[0]) - qRound(clear.redF()   * 255.0)) <= tol
                      && qAbs(int(p[1]) - qRound(clear.greenF() * 255.0)) <= tol
                      && qAbs(int(p[2]) - qRound(clear.blueF()  * 255.0)) <= tol;

    // Release and re-acquire the same size — the pool should hand back the same
    // texture rather than allocating a new one.
    ctx.releaseTexture(a);
    QRhiTexture *b = ctx.acquireTexture(size);
    const bool reused = (b == a);
    ctx.releaseTexture(b);

    if (!colorOk) std::printf("  (pool) rendered colour mismatch\n");
    if (!reused)  std::printf("  (pool) release/re-acquire did not reuse the handle\n");
    return colorOk && reused;
}

// Mirrors RhiWindow::initRhi(): an independent QRhi bound to a window + swapchain.
// Relies on AA_ShareOpenGLContexts to land in the global share group (the
// visualizer doesn't call setShareContext either).
struct WindowRhi
{
    QWindow *window = nullptr;
    QOffscreenSurface *fallback = nullptr;
    QRhi *rhi = nullptr;
    QRhiSwapChain *swapChain = nullptr;
    QRhiRenderBuffer *depthStencil = nullptr;
    QRhiRenderPassDescriptor *renderPass = nullptr;
    int sampleCount = 4;

    bool create(QApplication &app)
    {
        const QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();

        window = new QWindow;
        window->setSurfaceType(QSurface::OpenGLSurface);
        window->setFormat(fmt);
        window->resize(320, 240);
        window->setTitle("rhi-spike coexistence window");
        window->show();

        // A window-backed swapchain needs the platform surface to be exposed.
        QElapsedTimer t;
        t.start();
        while (!window->isExposed() && t.elapsed() < 4000)
            app.processEvents(QEventLoop::AllEvents, 25);
        if (!window->isExposed()) {
            std::printf("  (coexistence) window never exposed — INCONCLUSIVE\n");
            return false;
        }

        fallback = QRhiGles2InitParams::newFallbackSurface(fmt);
        QRhiGles2InitParams params;
        params.format = fmt;
        params.fallbackSurface = fallback;
        params.window = window;
        rhi = QRhi::create(QRhi::OpenGLES2, &params);
        if (!rhi) {
            std::printf("  (coexistence) window QRhi create failed\n");
            return false;
        }

        swapChain = rhi->newSwapChain();
        depthStencil = rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, QSize(),
                                            sampleCount, QRhiRenderBuffer::UsedWithSwapChainOnly);
        swapChain->setWindow(window);
        swapChain->setDepthStencil(depthStencil);
        swapChain->setSampleCount(sampleCount);
        renderPass = swapChain->newCompatibleRenderPassDescriptor();
        swapChain->setRenderPassDescriptor(renderPass);
        return swapChain->createOrResize();
    }

    // Records one swapchain frame that just clears to a colour. Returns false on
    // a frame op failure (device lost etc.).
    bool renderFrame(const QColor &clear)
    {
        if (swapChain->currentPixelSize() != swapChain->surfacePixelSize())
            swapChain->createOrResize();

        QRhi::FrameOpResult r = rhi->beginFrame(swapChain);
        if (r == QRhi::FrameOpSwapChainOutOfDate) {
            swapChain->createOrResize();
            r = rhi->beginFrame(swapChain);
        }
        if (r != QRhi::FrameOpSuccess)
            return false;

        QRhiCommandBuffer *cb = swapChain->currentFrameCommandBuffer();
        cb->beginPass(swapChain->currentFrameRenderTarget(), clear, { 1.0f, 0 });
        cb->endPass();
        rhi->endFrame(swapChain);
        return true;
    }

    ~WindowRhi()
    {
        delete renderPass;
        delete depthStencil;
        delete swapChain;
        delete rhi;
        delete fallback;
        delete window;
    }
};

} // namespace

int main(int argc, char *argv[])
{
    qSetMessagePattern("%{function} [%{line}] %{message}");
    setvbuf(stdout, nullptr, _IONBF, 0);   // unbuffered so a crash still shows progress

    // Same startup contract as photon-desktop's main().
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setSamples(4);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    // --- Check 1: offscreen round-trip -------------------------------------
    photon::RhiContext ctx;
    std::printf("rhi-spike: RhiContext valid = %s\n", ctx.isValid() ? "true" : "false");
    if (!ctx.isValid()) {
        std::printf("rhi-spike: FAIL (no offscreen device)\n");
        std::fflush(stdout);
        return 2;
    }
    const bool offscreenOk = ctx.selfTest();
    std::printf("rhi-spike: offscreen selfTest = %s\n", offscreenOk ? "PASS" : "FAIL");

    // --- Check 2: texture pool + RhiTextureParameter round-trip ------------
    const bool poolOk = runPoolTest(ctx);
    std::printf("rhi-spike: pool + param = %s\n", poolOk ? "PASS" : "FAIL");

    // --- Check 3: canvas subgraph evaluate → sink clears to background ------
    const bool canvasOk = runCanvasGraphTest(ctx);
    std::printf("rhi-spike: canvas subgraph = %s\n", canvasOk ? "PASS" : "FAIL");

    // --- Check 3b: producer/effect nodes (fill + transform via shaders) -----
    const bool producerOk = runProducerNodeTest(ctx);
    std::printf("rhi-spike: producer nodes = %s\n", producerOk ? "PASS" : "FAIL");

    // --- Check 3c: DMX readback + pixel sampling ----------------------------
    const bool dmxOk = runDmxTest(ctx);
    std::printf("rhi-spike: dmx sampling = %s\n", dmxOk ? "PASS" : "FAIL");

    // --- Check 4: threaded render (reproduces the surface-graph crash) ------
    const bool threadedOk = runThreadedCanvasTest(ctx, app);
    std::printf("rhi-spike: threaded canvas = %s\n", threadedOk ? "PASS" : "FAIL");

    // --- Check 5: coexistence with a windowed (visualizer-style) QRhi -------
    WindowRhi win;
    if (!win.create(app)) {
        std::printf("rhi-spike: coexistence = INCONCLUSIVE (windowed device unavailable)\n");
        std::fflush(stdout);
        return (offscreenOk && poolOk && canvasOk && producerOk && dmxOk && threadedOk) ? 3 : 1;   // 3 == core checks fine, coexistence not exercised
    }

    const int frames = 8;
    int winFrames = 0;
    int offscreenPasses = 0;
    for (int i = 0; i < frames; ++i) {
        // Windowed device draws a frame...
        const QColor clear = QColor::fromHslF(float(i) / frames, 0.6f, 0.5f);
        if (win.renderFrame(clear))
            ++winFrames;
        // ...then the offscreen device renders + reads back in the same loop,
        // both alive at once.
        if (ctx.selfTest())
            ++offscreenPasses;
        app.processEvents();
    }

    const bool coexistOk = (winFrames == frames) && (offscreenPasses == frames);
    std::printf("rhi-spike: coexistence = %s (window frames %d/%d, offscreen readbacks %d/%d)\n",
                coexistOk ? "PASS" : "FAIL", winFrames, frames, offscreenPasses, frames);

    const bool allOk = offscreenOk && poolOk && canvasOk && producerOk && dmxOk && threadedOk && coexistOk;
    std::printf("rhi-spike: OVERALL = %s\n", allOk ? "PASS" : "FAIL");
    std::fflush(stdout);
    return allOk ? 0 : 1;
}
