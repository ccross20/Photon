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
#include "graph/node/canvas/canvasnoisenode.h"
#include "graph/node/canvas/canvaslevelsnode.h"
#include "graph/node/canvas/canvascompositenode.h"
#include "graph/node/canvas/canvasshapenode.h"
#include "graph/node/canvas/canvastilenode.h"
#include "graph/node/canvas/canvasmasknode.h"
#include "graph/node/canvas/canvaswipenode.h"
#include "graph/node/canvas/canvasgradientnode.h"
#include "graph/node/canvas/canvasgradientmapnode.h"
#include "graph/parameter/gradientparameter.h"
#include "util/gradient.h"
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
    int pixelCount() const override { return 1; }
    QByteArray sourceUniqueId() const override { return "test"; }
    int dmxOffset() const override { return 0; }
    int universe() const override { return 1; }
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
    sampler.recordGather(ctx.rhi(), cb, fillOut.texture, uvs, QColor(0, 0, 0, 255), nullptr);
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
    QVector<QPointF> positions{ QPointF(0.5, 0.5) };
    src.process(pc, positions);
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
    // Sample the CENTRE texel (edge feathering means corner pixels aren't fully
    // covered for effects like Composite; all the uniform-fill tests are unaffected).
    const uchar *p = reinterpret_cast<const uchar *>(rb.data.constData());
    const int w = rb.pixelSize.width();
    const int h = rb.pixelSize.height();
    const int idx = ((h / 2) * w + w / 2) * 4;
    r = p[idx]; g = p[idx + 1]; b = p[idx + 2];
    return true;
}

// Reads the corner (0,0) texel's RGBA. Runs its own offscreen frame.
bool readCornerRGBA(QRhi *rhi, QRhiTexture *tex, int &r, int &g, int &b, int &a)
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
    r = p[0]; g = p[1]; b = p[2]; a = p[3];
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

// Renders the Noise node (fractal) and checks it produced greyscale output (the
// shader runs and outputs vec3(n)), and that panning the offset changes the value.
bool runNoiseNodeTest(photon::RhiContext &ctx)
{
    const QSize size(64, 64);

    photon::CanvasNoiseNode noise;
    noise.createParameters();
    noise.setValue(photon::CanvasNoiseNode::Mode, 1);      // Fractal
    noise.setValue(photon::CanvasNoiseNode::Scale, 8.0);

    int r = 0, g = 0, b = 0;
    if (!renderNodeAndReadback(ctx, &noise, photon::CanvasNoiseNode::Output, size, r, g, b)) {
        std::printf("  (noise) render/readback failed\n");
        return false;
    }
    if (r != g || g != b) {   // greyscale
        std::printf("  (noise) not greyscale: %d %d %d\n", r, g, b);
        return false;
    }

    // Pan the noise; the sampled value should change.
    noise.setValue(photon::CanvasNoiseNode::Offset, QPointF{37.0,0.0});
    int r2 = 0, g2 = 0, b2 = 0;
    if (!renderNodeAndReadback(ctx, &noise, photon::CanvasNoiseNode::Output, size, r2, g2, b2)) {
        std::printf("  (noise) second render failed\n");
        return false;
    }
    if (r2 == r) {
        std::printf("  (noise) offset had no effect (%d)\n", r);
        return false;
    }
    return true;
}

// Fills mid-grey, runs it through Levels with input-white pulled down to 0.5, which
// should push 0.5 up to full white — verifies the effect samples + remaps.
bool runLevelsNodeTest(photon::RhiContext &ctx)
{
    const QSize size(64, 64);
    const QColor grey(128, 128, 128, 255);   // ~0.5

    photon::CanvasFillNode fillNode;
    fillNode.createParameters();
    fillNode.setValue(photon::CanvasFillNode::Color1, QVariant(grey));
    fillNode.setValue(photon::CanvasFillNode::Mode, 0);

    int r = 0, g = 0, b = 0;
    if (!renderNodeAndReadback(ctx, &fillNode, photon::CanvasFillNode::Output, size, r, g, b))
        return false;
    const auto fillOut = fillNode.findParameter(photon::CanvasFillNode::Output)
                             ->value().value<photon::RhiTextureData>();

    photon::CanvasLevelsNode levels;
    levels.createParameters();
    levels.setValue(photon::CanvasLevelsNode::Input, QVariant::fromValue(fillOut));
    levels.setValue(photon::CanvasLevelsNode::InputWhite, 0.5);   // 0.5 -> 1.0

    int lr = 0, lg = 0, lb = 0;
    if (!renderNodeAndReadback(ctx, &levels, photon::CanvasLevelsNode::Output, size, lr, lg, lb)) {
        std::printf("  (levels) render/readback failed\n");
        return false;
    }
    const bool ok = lr >= 250 && lg >= 250 && lb >= 250;   // ~white
    if (!ok)
        std::printf("  (levels) expected ~white, got %d %d %d\n", lr, lg, lb);
    return ok;
}

// Composites a green top over a red base with Add blend (identity transform) —
// the whole canvas should come out yellow, verifying both inputs are sampled.
bool runCompositeNodeTest(photon::RhiContext &ctx)
{
    const QSize size(64, 64);

    photon::CanvasFillNode fillRed;
    fillRed.createParameters();
    fillRed.setValue(photon::CanvasFillNode::Color1, QVariant(QColor(255, 0, 0, 255)));
    fillRed.setValue(photon::CanvasFillNode::Mode, 0);

    photon::CanvasFillNode fillGreen;
    fillGreen.createParameters();
    fillGreen.setValue(photon::CanvasFillNode::Color1, QVariant(QColor(0, 255, 0, 255)));
    fillGreen.setValue(photon::CanvasFillNode::Mode, 0);

    int t;
    renderNodeAndReadback(ctx, &fillRed, photon::CanvasFillNode::Output, size, t, t, t);
    renderNodeAndReadback(ctx, &fillGreen, photon::CanvasFillNode::Output, size, t, t, t);
    const auto redOut = fillRed.findParameter(photon::CanvasFillNode::Output)->value().value<photon::RhiTextureData>();
    const auto greenOut = fillGreen.findParameter(photon::CanvasFillNode::Output)->value().value<photon::RhiTextureData>();

    photon::CanvasCompositeNode comp;
    comp.createParameters();
    comp.setValue(photon::CanvasCompositeNode::Base, QVariant::fromValue(redOut));
    comp.setValue(photon::CanvasCompositeNode::Top, QVariant::fromValue(greenOut));
    comp.setValue(photon::CanvasCompositeNode::BlendMode, 1);   // Add

    int r = 0, g = 0, b = 0;
    if (!renderNodeAndReadback(ctx, &comp, photon::CanvasCompositeNode::Output, size, r, g, b)) {
        std::printf("  (composite) render/readback failed\n");
        return false;
    }
    const int tol = 3;
    const bool centreOk = r >= 255 - tol && g >= 255 - tol && b <= tol;   // yellow
    if (!centreOk)
        std::printf("  (composite) expected yellow centre, got %d %d %d\n", r, g, b);

    // With an identity top transform the top fills the canvas, so even the CORNER
    // texel must be full yellow — no 1px base (red) bleed-through from edge AA.
    const auto compOut = comp.findParameter(photon::CanvasCompositeNode::Output)
                             ->value().value<photon::RhiTextureData>();
    int cr = 0, cg = 0, cb = 0, ca = 0;
    if (compOut.texture && readCornerRGBA(ctx.rhi(), compOut.texture, cr, cg, cb, ca)) {
        const bool cornerOk = cr >= 255 - tol && cg >= 255 - tol && cb <= tol;
        if (!cornerOk)
            std::printf("  (composite) corner not full yellow (base bleed): %d %d %d\n", cr, cg, cb);
        return centreOk && cornerOk;
    }
    return centreOk;
}

// Reads the centre texel's RGBA. Runs its own offscreen frame.
bool readCenterRGBA(QRhi *rhi, QRhiTexture *tex, int &r, int &g, int &b, int &a)
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
    const int idx = ((rb.pixelSize.height() / 2) * rb.pixelSize.width() + rb.pixelSize.width() / 2) * 4;
    r = p[idx]; g = p[idx + 1]; b = p[idx + 2]; a = p[idx + 3];
    return true;
}

// Renders a circle: opaque at the centre, and transparent there once moved off
// the canvas — verifying the SDF actually cuts the shape (in the alpha channel).
bool runShapeNodeTest(photon::RhiContext &ctx)
{
    const QSize size(64, 64);

    photon::CanvasShapeNode shape;
    shape.createParameters();
    shape.setValue(photon::CanvasShapeNode::Shape, 0);   // Circle

    auto renderOnce = [&](int &r, int &g, int &b, int &a) -> bool {
        QRhiCommandBuffer *cb = nullptr;
        if (ctx.rhi()->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess || !cb)
            return false;
        photon::DMXMatrix m;
        photon::RoutineEvaluationContext c(m);
        c.rhiContext = &ctx;
        c.rhiCommandBuffer = cb;
        c.canvasResolution = size;
        shape.evaluate(&c);
        ctx.rhi()->endOffscreenFrame();
        const auto out = shape.findParameter(photon::CanvasShapeNode::Output)
                             ->value().value<photon::RhiTextureData>();
        return out.texture && readCenterRGBA(ctx.rhi(), out.texture, r, g, b, a);
    };

    int r = 0, g = 0, b = 0, a = 0;
    if (!renderOnce(r, g, b, a)) {
        std::printf("  (shape) render failed\n");
        return false;
    }
    const bool insideOk = a >= 250 && r >= 250 && g >= 250 && b >= 250;   // opaque white centre
    if (!insideOk)
        std::printf("  (shape) centre not opaque white: %d %d %d a=%d\n", r, g, b, a);

    shape.setValue(photon::CanvasShapeNode::Position, QVariant(QPointF(3.0, 3.0)));   // off-canvas
    int a2 = 255;
    if (!renderOnce(r, g, b, a2))
        return false;
    const bool outsideOk = a2 <= 5;   // centre now outside the shape
    if (!outsideOk)
        std::printf("  (shape) centre not transparent when shape moved away: a=%d\n", a2);

    // Stroke: a radius-0 circle with a transparent fill and a wide red stroke should
    // paint the centre (on the boundary) with the stroke colour.
    shape.setValue(photon::CanvasShapeNode::Position, QVariant(QPointF(0.5, 0.5)));
    shape.setValue(photon::CanvasShapeNode::Size, QVariant(QPointF(0.0, 0.0)));
    shape.setValue(photon::CanvasShapeNode::Fill, QVariant(QColor(255, 255, 255, 0)));
    shape.setValue(photon::CanvasShapeNode::StrokeColor, QVariant(QColor(255, 0, 0, 255)));
    shape.setValue(photon::CanvasShapeNode::StrokeWidth, 0.3);
    int sr = 0, sg = 0, sb = 0, sa = 0;
    if (!renderOnce(sr, sg, sb, sa))
        return false;
    const bool strokeOk = sr >= 250 && sg <= 5 && sb <= 5 && sa >= 250;
    if (!strokeOk)
        std::printf("  (shape) stroke colour mismatch: %d %d %d a=%d\n", sr, sg, sb, sa);

    return insideOk && outsideOk && strokeOk;
}

// Tiles a horizontal black->white gradient 2x. Repeat wraps the centre back to the
// black (left) edge -> dark; mirror folds it to the white edge -> bright.
bool runTileNodeTest(photon::RhiContext &ctx)
{
    const QSize size(64, 64);

    photon::CanvasFillNode grad;
    grad.createParameters();
    grad.setValue(photon::CanvasFillNode::Color1, QVariant(QColor(0, 0, 0, 255)));
    grad.setValue(photon::CanvasFillNode::Color2, QVariant(QColor(255, 255, 255, 255)));
    grad.setValue(photon::CanvasFillNode::Mode, 2);   // horizontal gradient

    int t;
    renderNodeAndReadback(ctx, &grad, photon::CanvasFillNode::Output, size, t, t, t);
    const auto gradOut = grad.findParameter(photon::CanvasFillNode::Output)->value().value<photon::RhiTextureData>();

    photon::CanvasTileNode tile;
    tile.createParameters();
    tile.setValue(photon::CanvasTileNode::Input, QVariant::fromValue(gradOut));
    tile.setValue(photon::CanvasTileNode::Tiles, QVariant(QPointF(2.0, 1.0)));

    int rr = 0, g = 0, b = 0;
    if (!renderNodeAndReadback(ctx, &tile, photon::CanvasTileNode::Output, size, rr, g, b)) {
        std::printf("  (tile) repeat render failed\n");
        return false;
    }

    tile.setValue(photon::CanvasTileNode::MirrorX, QVariant(true));
    int mr = 0;
    if (!renderNodeAndReadback(ctx, &tile, photon::CanvasTileNode::Output, size, mr, g, b)) {
        std::printf("  (tile) mirror render failed\n");
        return false;
    }

    const bool ok = rr < 40 && mr > 210;   // repeat -> dark centre, mirror -> bright
    if (!ok)
        std::printf("  (tile) expected repeat dark / mirror bright, got %d / %d\n", rr, mr);
    return ok;
}

// Masks a red source with a 50%-alpha mask: the output should stay red but at ~50%
// alpha (source.a * mask.a).
bool runMaskNodeTest(photon::RhiContext &ctx)
{
    const QSize size(64, 64);

    photon::CanvasFillNode src;
    src.createParameters();
    src.setValue(photon::CanvasFillNode::Color1, QVariant(QColor(255, 0, 0, 255)));
    src.setValue(photon::CanvasFillNode::Mode, 0);

    photon::CanvasFillNode maskFill;
    maskFill.createParameters();
    maskFill.setValue(photon::CanvasFillNode::Color1, QVariant(QColor(255, 255, 255, 128)));   // 50% alpha
    maskFill.setValue(photon::CanvasFillNode::Mode, 0);

    int t;
    renderNodeAndReadback(ctx, &src, photon::CanvasFillNode::Output, size, t, t, t);
    renderNodeAndReadback(ctx, &maskFill, photon::CanvasFillNode::Output, size, t, t, t);
    const auto srcOut = src.findParameter(photon::CanvasFillNode::Output)->value().value<photon::RhiTextureData>();
    const auto maskOut = maskFill.findParameter(photon::CanvasFillNode::Output)->value().value<photon::RhiTextureData>();

    photon::CanvasMaskNode mask;
    mask.createParameters();
    mask.setValue(photon::CanvasMaskNode::Source, QVariant::fromValue(srcOut));
    mask.setValue(photon::CanvasMaskNode::Mask, QVariant::fromValue(maskOut));   // channel = Alpha (default)

    QRhiCommandBuffer *cb = nullptr;
    if (ctx.rhi()->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess || !cb)
        return false;
    photon::DMXMatrix m;
    photon::RoutineEvaluationContext c(m);
    c.rhiContext = &ctx;
    c.rhiCommandBuffer = cb;
    c.canvasResolution = size;
    mask.evaluate(&c);
    ctx.rhi()->endOffscreenFrame();

    const auto out = mask.findParameter(photon::CanvasMaskNode::Output)->value().value<photon::RhiTextureData>();
    int r = 0, g = 0, b = 0, a = 0;
    if (!out.texture || !readCenterRGBA(ctx.rhi(), out.texture, r, g, b, a)) {
        std::printf("  (mask) render/readback failed\n");
        return false;
    }
    const bool ok = r >= 250 && g <= 5 && b <= 5 && a >= 118 && a <= 138;
    if (!ok)
        std::printf("  (mask) expected red at ~50%% alpha, got %d %d %d a=%d\n", r, g, b, a);
    return ok;
}

// Linear wipe between a red "From" and green "To": progress 0 -> all From (red),
// progress 1 -> all To (green).
bool runWipeNodeTest(photon::RhiContext &ctx)
{
    const QSize size(64, 64);

    photon::CanvasFillNode fromFill;
    fromFill.createParameters();
    fromFill.setValue(photon::CanvasFillNode::Color1, QVariant(QColor(255, 0, 0, 255)));
    fromFill.setValue(photon::CanvasFillNode::Mode, 0);

    photon::CanvasFillNode toFill;
    toFill.createParameters();
    toFill.setValue(photon::CanvasFillNode::Color1, QVariant(QColor(0, 255, 0, 255)));
    toFill.setValue(photon::CanvasFillNode::Mode, 0);

    int t;
    renderNodeAndReadback(ctx, &fromFill, photon::CanvasFillNode::Output, size, t, t, t);
    renderNodeAndReadback(ctx, &toFill, photon::CanvasFillNode::Output, size, t, t, t);
    const auto fromOut = fromFill.findParameter(photon::CanvasFillNode::Output)->value().value<photon::RhiTextureData>();
    const auto toOut = toFill.findParameter(photon::CanvasFillNode::Output)->value().value<photon::RhiTextureData>();

    photon::CanvasWipeNode wipe;
    wipe.createParameters();
    wipe.setValue(photon::CanvasWipeNode::From, QVariant::fromValue(fromOut));
    wipe.setValue(photon::CanvasWipeNode::To, QVariant::fromValue(toOut));

    wipe.setValue(photon::CanvasWipeNode::Progress, 0.0);
    int r0 = 0, g0 = 0, b0 = 0;
    if (!renderNodeAndReadback(ctx, &wipe, photon::CanvasWipeNode::Output, size, r0, g0, b0))
        return false;

    wipe.setValue(photon::CanvasWipeNode::Progress, 1.0);
    int r1 = 0, g1 = 0, b1 = 0;
    if (!renderNodeAndReadback(ctx, &wipe, photon::CanvasWipeNode::Output, size, r1, g1, b1))
        return false;

    const bool ok = r0 > 200 && g0 < 40 && g1 > 200 && r1 < 40;   // red at 0, green at 1
    if (!ok)
        std::printf("  (wipe) expected red@0 / green@1, got (%d,%d) / (%d,%d)\n", r0, g0, r1, g1);
    return ok;
}

// Linear gradient source node: a 3-stop black -> red -> black gradient (angle 0)
// should read red at the horizontal centre (t = 0.5 lands exactly on the middle
// stop). Verifies stop-lookup + uniform packing in the shader.
bool runGradientNodeTest(photon::RhiContext &ctx)
{
    const QSize size(64, 64);

    photon::CanvasGradientNode grad;
    grad.createParameters();
    grad.setValue(photon::CanvasGradientNode::Type, 0);   // linear
    grad.setValue(photon::CanvasGradientNode::Angle, 0.0);

    photon::Gradient g({ {0.0, QColor(0, 0, 0, 255)},
                         {0.5, QColor(255, 0, 0, 255)},
                         {1.0, QColor(0, 0, 0, 255)} });
    grad.setValue(photon::CanvasGradientNode::GradientId, QVariant::fromValue(g));

    int r = 0, gr = 0, b = 0;
    if (!renderNodeAndReadback(ctx, &grad, photon::CanvasGradientNode::Output, size, r, gr, b)) {
        std::printf("  (gradient) render/readback failed\n");
        return false;
    }
    const bool ok = r > 200 && gr < 40 && b < 40;   // centre stop is red
    if (!ok)
        std::printf("  (gradient) expected red at centre, got %d %d %d\n", r, gr, b);
    return ok;
}

// Gradient map: a white fill (luminance 1) mapped through a black->red gradient
// should come out red, verifying the channel lookup + gradient sampling.
bool runGradientMapNodeTest(photon::RhiContext &ctx)
{
    const QSize size(64, 64);

    photon::CanvasFillNode fillNode;
    fillNode.createParameters();
    fillNode.setValue(photon::CanvasFillNode::Color1, QVariant(QColor(255, 255, 255, 255)));
    fillNode.setValue(photon::CanvasFillNode::Mode, 0);

    int t;
    renderNodeAndReadback(ctx, &fillNode, photon::CanvasFillNode::Output, size, t, t, t);
    const auto fillOut = fillNode.findParameter(photon::CanvasFillNode::Output)
                             ->value().value<photon::RhiTextureData>();

    photon::CanvasGradientMapNode gmap;
    gmap.createParameters();
    gmap.setValue(photon::CanvasGradientMapNode::Source, QVariant::fromValue(fillOut));
    photon::Gradient g({ {0.0, QColor(0, 0, 0, 255)}, {1.0, QColor(255, 0, 0, 255)} });
    gmap.setValue(photon::CanvasGradientMapNode::GradientId, QVariant::fromValue(g));

    int r = 0, gr = 0, b = 0;
    if (!renderNodeAndReadback(ctx, &gmap, photon::CanvasGradientMapNode::Output, size, r, gr, b)) {
        std::printf("  (gradient map) render/readback failed\n");
        return false;
    }
    const bool ok = r > 200 && gr < 40 && b < 40;   // white luminance -> gradient top = red
    if (!ok)
        std::printf("  (gradient map) expected red, got %d %d %d\n", r, gr, b);
    return ok;
}

// CPU-only checks for the Gradient value type's reverse / remap / mix / sample
// operations that back the gradient helper nodes.
bool runGradientHelpersTest()
{
    const auto near = [](const QColor &c, int r, int g, int b, int tol = 3){
        return qAbs(c.red() - r) <= tol && qAbs(c.green() - g) <= tol && qAbs(c.blue() - b) <= tol;
    };

    photon::Gradient bw({ {0.0, QColor(0, 0, 0)}, {1.0, QColor(255, 255, 255)} });

    // Sample: midpoint of black -> white is grey.
    if (!near(bw.colorAt(0.5), 128, 128, 128)) {
        std::printf("  (helpers) sample midpoint wrong: %d\n", bw.colorAt(0.5).red());
        return false;
    }

    // Reverse: ends swap.
    const photon::Gradient rev = bw.reversed();
    if (!near(rev.colorAt(0.0), 255, 255, 255) || !near(rev.colorAt(1.0), 0, 0, 0)) {
        std::printf("  (helpers) reverse wrong\n");
        return false;
    }

    // Mix of a gradient with its reverse at 0.5 is grey everywhere.
    const photon::Gradient mix = photon::Gradient::mixed(bw, rev, 0.5);
    if (!near(mix.colorAt(0.0), 128, 128, 128) || !near(mix.colorAt(1.0), 128, 128, 128)) {
        std::printf("  (helpers) mix wrong: %d / %d\n", mix.colorAt(0.0).red(), mix.colorAt(1.0).red());
        return false;
    }

    // Remap repeat, offset 0.5: the seam colour (source midpoint = grey) lands at 0.
    const photon::Gradient shifted = bw.remapped(0.5, 1.0, true);
    if (!near(shifted.colorAt(0.0), 128, 128, 128, 6)) {
        std::printf("  (helpers) remap offset wrong: %d\n", shifted.colorAt(0.0).red());
        return false;
    }

    return true;
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

    const bool noiseOk = runNoiseNodeTest(ctx);
    std::printf("rhi-spike: noise node = %s\n", noiseOk ? "PASS" : "FAIL");

    const bool levelsOk = runLevelsNodeTest(ctx);
    std::printf("rhi-spike: levels node = %s\n", levelsOk ? "PASS" : "FAIL");

    const bool compositeOk = runCompositeNodeTest(ctx);
    std::printf("rhi-spike: composite node = %s\n", compositeOk ? "PASS" : "FAIL");

    const bool shapeOk = runShapeNodeTest(ctx);
    std::printf("rhi-spike: shape node = %s\n", shapeOk ? "PASS" : "FAIL");

    const bool tileOk = runTileNodeTest(ctx);
    std::printf("rhi-spike: tile node = %s\n", tileOk ? "PASS" : "FAIL");

    const bool maskOk = runMaskNodeTest(ctx);
    std::printf("rhi-spike: mask node = %s\n", maskOk ? "PASS" : "FAIL");

    const bool wipeOk = runWipeNodeTest(ctx);
    std::printf("rhi-spike: wipe node = %s\n", wipeOk ? "PASS" : "FAIL");

    const bool gradientOk = runGradientNodeTest(ctx);
    std::printf("rhi-spike: gradient node = %s\n", gradientOk ? "PASS" : "FAIL");

    const bool gradHelpersOk = runGradientHelpersTest();
    std::printf("rhi-spike: gradient helpers = %s\n", gradHelpersOk ? "PASS" : "FAIL");

    const bool gradMapOk = runGradientMapNodeTest(ctx);
    std::printf("rhi-spike: gradient map = %s\n", gradMapOk ? "PASS" : "FAIL");

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
        return (offscreenOk && poolOk && canvasOk && producerOk && noiseOk && levelsOk && compositeOk && shapeOk && tileOk && maskOk && wipeOk && gradientOk && gradHelpersOk && gradMapOk && dmxOk && threadedOk) ? 3 : 1;   // 3 == core checks fine, coexistence not exercised
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

    const bool allOk = offscreenOk && poolOk && canvasOk && producerOk && noiseOk && levelsOk && compositeOk && shapeOk && tileOk && maskOk && wipeOk && gradientOk && gradHelpersOk && gradMapOk && dmxOk && threadedOk && coexistOk;
    std::printf("rhi-spike: OVERALL = %s\n", allOk ? "PASS" : "FAIL");
    std::fflush(stdout);
    return allOk ? 0 : 1;
}
