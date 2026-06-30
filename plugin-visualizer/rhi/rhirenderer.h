#ifndef PHOTON_RHIRENDERER_H
#define PHOTON_RHIRENDERER_H

#include <QVector>
#include <QMatrix4x4>
#include <QVector4D>
#include <QVector3D>
#include <QColor>
#include <QHash>
#include <QPair>
#include <QByteArray>
#include <QSet>
#include <QElapsedTimer>
#include <QImage>
#include "rhigizmo.h"
#include "rhimodel.h"
#include "data/dmxmatrix.h"

class QRhi;
class QRhiBuffer;
class QRhiGraphicsPipeline;
class QRhiShaderResourceBindings;
class QRhiRenderTarget;
class QRhiRenderPassDescriptor;
class QRhiCommandBuffer;
class QRhiTexture;
class QRhiSampler;
class QRhiResourceUpdateBatch;

namespace photon {

class SceneObject;
class Fixture;
class FixtureChannel;
class RhiMesh;
class RhiCamera;

// Owns all QRhi GPU resources for the visualizer and records the frame.
// Milestone A: a ground grid plus one lit box per fixture in the scene tree.
class RhiRenderer
{
public:
    // Light-beam rendering model.
    enum class BeamMode {
        Basic,        // cheap additive translucent cone
        Volumetric    // analytic single-scatter raymarch through the cone
    };

    RhiRenderer();
    ~RhiRenderer();

    void setSceneRoot(SceneObject *root) {
        m_sceneRoot = root;
        m_typePath.clear();       // re-resolve model files (e.g. newly added ones)
        m_emitterWorld.clear();
    }

    // Latest evaluated DMX output. Drives live fixture color/intensity and beams.
    void setDmxState(const DMXMatrix &dmx) { m_dmx = dmx; }

    void setBeamMode(BeamMode mode) { m_beamMode = mode; }
    BeamMode beamMode() const { return m_beamMode; }

    // Global gobo selection for fixtures with no gobo wheel (0 = none, 1..goboCount()
    // selects a loaded gobo texture layer).
    void setGoboIndex(int index) { m_goboIndex = index; }
    int goboIndex() const { return m_goboIndex; }
    int goboCount() const { return m_goboImages.size(); }

    // (Re)creates device resources. Pass a render pass descriptor compatible with
    // the target that will be drawn to (e.g. the swapchain's), and its sample count.
    void initialize(QRhi *rhi, QRhiRenderPassDescriptor *rpDesc, int sampleCount);
    // Releases all device resources (device loss / teardown).
    void releaseResources();

    void render(QRhiCommandBuffer *cb, QRhiRenderTarget *rt, const RhiCamera &camera);

    // Ray-pick the nearest selectable scene object (world-space ray). Null = miss.
    SceneObject *pick(const QVector3D &origin, const QVector3D &dir) const;
    void setSelection(SceneObject *obj);
    SceneObject *selection() const { return m_selected; }

    RhiGizmo &gizmo() { return m_gizmo; }

private:
    struct Drawable {
        RhiMesh   *mesh;   // non-owning; shared box or a cached per-truss mesh
        QMatrix4x4 model;
        QColor     color;
        bool       emissive = false; // meshes: render unlit/full-bright (e.g. lens face)
        float      gobo = 0.0f;     // beams only: gobo layer (0 = none, 1..N)
        float      gobo2 = 0.0f;    // beams only: second gobo layer (wheel wipe)
        float      goboSplit = -2.0f; // beams only: gobo wipe boundary x in [-1,1]
        float      goboRot = 0.0f;  // beams only: gobo rotation (radians)
        QColor     color2;          // beams only: second color-wheel color (split)
        float      split = -2.0f;   // beams only: color split boundary x in [-1,1] (<-1 = none)
        QVector4D  fadePlane;       // volumetric beams: xyz = plane normal (toward apex),
                                    // w = offset (signed dist = dot(n,X)+w); zero = no fade
    };

    void collectDrawables(SceneObject *obj, QVector<Drawable> &out,
                          QSet<SceneObject *> &seenTrusses);
    // Appends a beam-cone drawable for every lit fixture in the subtree.
    void collectBeams(SceneObject *obj, QVector<Drawable> &out) const;
    // Appends a lit-plane drawable for every wall/floor surface in the subtree.
    void collectSurfaces(SceneObject *obj, QVector<Drawable> &out) const;
    // Appends wireframe-box line verts (pos+color, 6 floats each) for every SceneZone
    // in the subtree, into the gizmo line buffer (drawn as a depth-off overlay).
    void appendZoneWireframes(SceneObject *obj, QByteArray &out) const;
    // Collects every surface plane (world point + unit normal) for volumetric beam
    // soft-fade against opaque surfaces. Rebuilt each frame before collectBeams.
    void gatherSurfacePlanes(SceneObject *obj) const;
    // For a beam (apex + downward axis), the plane it should fade against: the nearest
    // surface its axis crosses within reach. Returns zero (no fade) when none.
    QVector4D fadePlaneFor(const QVector3D &apex, const QVector3D &axis, float length) const;
    RhiMesh *trussMeshFor(SceneObject *truss);

    // Reads the fixture's live color and 0..1 intensity from the current DMX state.
    // Returns false if the fixture has no color/dimmer capability to read.
    bool evaluateFixture(Fixture *fixture, QColor &outColor, float &outIntensity) const;

    // Current shutter/strobe output multiplier (0..1) from the active shutter channel:
    // 1 when open or absent, 0 when closed, a time-driven flash/pulse when strobing.
    float shutterFactor(Fixture *fixture) const;

    // Live beam cone half-angle (degrees), driven by the zoom channel + lens range
    // when present, otherwise the default reference angle.
    float beamHalfAngleFor(Fixture *fixture) const;

    // Resolves a fixture's gobo wheel to the layer(s) in the gate. As the wheel
    // slews between slots two layers straddle the gate (a wipe), giving outA != outB
    // and a split boundary; parked on a slot, outA == outB. Layer 0 = no gobo.
    void goboForFixture(Fixture *fixture, float &outA, float &outB, float &outSplit) const;

    // Current gobo rotation (radians) from the fixture's gobo-rotation channel:
    // continuous speed is accumulated over time; indexed mode returns a static angle.
    float goboRotationFor(Fixture *fixture) const;

    // Current prism rotation (degrees) from the prism-rotation channel paired with the
    // engaged prism (channelIndex = its index among prism-rotation channels; -1 = any).
    float prismRotationFor(Fixture *fixture, int channelIndex = -1) const;

    // Advances a fixture's motorised attributes (pan/tilt + zoom) toward their DMX
    // targets over time and returns the current smoothed values.
    void updateFixtureMotion(Fixture *fixture, float &panOut, float &tiltOut,
                             float &halfAngleOut) const;
    // Advances motion for every fixture once per frame (so the model body and the
    // beam read the same smoothed values without double-advancing).
    void updateMotionPass(SceneObject *obj) const;

    // Resolves + loads the 3D model for a fixture (cached), or null for the box.
    RhiModel *modelForFixture(Fixture *fixture) const;
    QString resolveModelPath(Fixture *fixture) const;
    // World placement for a fixture's model root: the fixture's scene transform, offset
    // so the model's "origin" null (if any) coincides with it.
    QMatrix4x4 fixtureModelMatrix(Fixture *fixture, RhiModel *model) const;
    // Walks a model node tree, emitting a drawable per mesh (pan/tilt applied at the
    // rigged joints) and capturing the "lamp" emitter's world transform.
    void collectModelNodes(const RhiModel::Node &node, const QMatrix4x4 &parentWorld,
                           float pan, float tilt, bool selected, const QColor &lensColor,
                           QVector<Drawable> &out, QMatrix4x4 *emitterWorld) const;

    // Resolves a fixture's color wheel to the colour(s) visible in the gate. Returns
    // false if the fixture has no active colour wheel (open / RGB mixing only). When
    // the gate straddles two slots (or the wheel is spinning) outA/outB differ and
    // outSplit is the boundary position across the beam (-1..1).
    bool colorWheelFor(Fixture *fixture, QColor &outA, QColor &outB, float &outSplit) const;

    // Loads gobo PNGs from the deployed fixtures/gobos/ folder into m_goboImages.
    void loadGoboImages();
    void ensureObjectBuffer(int count);
    void ensureBeamBuffer(int count);

    static bool localBounds(SceneObject *obj, QVector3D &outMin, QVector3D &outMax);
    void pickRecursive(SceneObject *obj, const QVector3D &origin, const QVector3D &dir,
                       SceneObject *&best, float &bestT) const;

    QRhi *m_rhi = nullptr;

    RhiMesh *m_box = nullptr;
    RhiMesh *m_grid = nullptr;
    RhiMesh *m_beamCone = nullptr;
    RhiMesh *m_plane = nullptr;

    // Per-truss geometry, rebuilt when a truss's parameters change.
    QHash<SceneObject *, RhiMesh *>  m_trussMeshes;
    QHash<SceneObject *, QByteArray> m_trussSig;

    QRhiBuffer *m_frameBuffer = nullptr;   // per-frame constants (binding 0)
    QRhiBuffer *m_objectBuffer = nullptr;  // per-object, dynamic offset (binding 1)
    int         m_objectCapacity = 0;
    quint32     m_objectSlotSize = 0;

    // Per-beam uniforms for the volumetric pass (binding 1): model, color, and the
    // cone parameters the raymarch needs (apex, axis, cos half-angle, length).
    QRhiBuffer *m_beamBuffer = nullptr;
    int         m_beamCapacity = 0;
    quint32     m_beamSlotSize = 0;

    // Spotlight list for surface lighting (rebuilt each frame from lit fixtures).
    QRhiBuffer *m_lightsBuffer = nullptr;

    // Gobo texture array (one layer per loaded image), sampled by beams + surfaces.
    QVector<QImage> m_goboImages;
    // Lowercase gobo filename -> 1-based texture layer, for per-slot image lookup.
    QHash<QString, int> m_goboFileLayer;
    QRhiTexture *m_goboTex = nullptr;
    QRhiSampler *m_goboSampler = nullptr;
    bool m_goboUploaded = false;

    QRhiShaderResourceBindings *m_meshSrb = nullptr;
    QRhiShaderResourceBindings *m_lineSrb = nullptr;
    QRhiShaderResourceBindings *m_beamSrb = nullptr;     // frame + per-beam params
    QRhiShaderResourceBindings *m_surfaceSrb = nullptr;  // frame + object + lights
    QRhiGraphicsPipeline *m_meshPipeline = nullptr;
    QRhiGraphicsPipeline *m_surfacePipeline = nullptr;   // spotlight-lit planes
    QRhiGraphicsPipeline *m_linePipeline = nullptr;
    QRhiGraphicsPipeline *m_beamPipeline = nullptr;    // basic: triangles, additive, depth-write off
    QRhiGraphicsPipeline *m_beamVolPipeline = nullptr; // volumetric raymarch
    QRhiGraphicsPipeline *m_gizmoPipeline = nullptr;   // lines, depth test off
    QRhiBuffer *m_gizmoBuffer = nullptr;               // dynamic, rebuilt per frame

    // Fixture models, deduped by file path; type->path resolution cached separately
    // (keyed by model type, so changing a fixture's type re-resolves automatically).
    mutable QHash<QString, RhiModel *> m_models;
    mutable QHash<QString, QString> m_typePath;
    // Lamp-emitter world transform per lit fixture with a model (beam origin).
    QHash<SceneObject *, QMatrix4x4> m_emitterWorld;
    // Surface planes (world point, unit normal) for volumetric beam soft-fade.
    mutable QVector<QPair<QVector3D, QVector3D>> m_surfacePlanes;

    RhiGizmo     m_gizmo;
    SceneObject *m_sceneRoot = nullptr;
    SceneObject *m_selected = nullptr;
    BeamMode     m_beamMode = BeamMode::Basic;
    int          m_goboIndex = 0;
    QElapsedTimer m_clock;   // drives gobo rotation
    mutable double m_lastClockSec = 0.0;
    mutable float  m_frameDt = 0.0f;                   // seconds since last frame
    mutable QHash<SceneObject *, float> m_goboPhase;   // accumulated continuous rotation
    mutable QHash<SceneObject *, float> m_colorPhase;  // accumulated color-wheel position
    mutable QHash<FixtureChannel *, float> m_prismPhase;  // accumulated prism rotation (deg), per rotation channel

    // Per-fixture smoothed motor state (degrees) + velocities, for motor-like movement.
    struct FixtureMotion {
        float pan = 0.0f, panVel = 0.0f;
        float tilt = 0.0f, tiltVel = 0.0f;
        float zoom = 0.0f, zoomVel = 0.0f;   // beam half-angle (deg)
        bool  init = false;
    };
    mutable QHash<SceneObject *, FixtureMotion> m_motion;
    mutable QHash<SceneObject *, float> m_goboWheelPos;  // smoothed gobo-wheel layer position

    DMXMatrix m_dmx;   // latest evaluated DMX output

};

} // namespace photon

#endif // PHOTON_RHIRENDERER_H
