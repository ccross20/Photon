#ifndef PHOTON_RHIRENDERER_H
#define PHOTON_RHIRENDERER_H

#include <QVector>
#include <QMatrix4x4>
#include <QColor>
#include <QHash>
#include <QByteArray>
#include <QSet>
#include <QElapsedTimer>
#include <QImage>
#include "rhigizmo.h"
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

    void setSceneRoot(SceneObject *root) { m_sceneRoot = root; }

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
        float      gobo = 0.0f;     // beams only: gobo pattern index (0 = none)
        float      goboRot = 0.0f;  // beams only: gobo rotation (radians)
        QColor     color2;          // beams only: second color-wheel color (split)
        float      split = -2.0f;   // beams only: split boundary x in [-1,1] (<-1 = none)
    };

    void collectDrawables(SceneObject *obj, QVector<Drawable> &out,
                          QSet<SceneObject *> &seenTrusses);
    // Appends a beam-cone drawable for every lit fixture in the subtree.
    void collectBeams(SceneObject *obj, QVector<Drawable> &out) const;
    // Appends a lit-plane drawable for every wall/floor surface in the subtree.
    void collectSurfaces(SceneObject *obj, QVector<Drawable> &out) const;
    RhiMesh *trussMeshFor(SceneObject *truss);

    // Reads the fixture's live color and 0..1 intensity from the current DMX state.
    // Returns false if the fixture has no color/dimmer capability to read.
    bool evaluateFixture(Fixture *fixture, QColor &outColor, float &outIntensity) const;

    // Live beam cone half-angle (degrees), driven by the zoom channel + lens range
    // when present, otherwise the default reference angle.
    float beamHalfAngleFor(Fixture *fixture) const;

    // Active gobo pattern for a fixture: its gobo-wheel slot if it has one,
    // otherwise the global test-gobo selection.
    float goboForFixture(Fixture *fixture) const;

    // Current gobo rotation (radians) from the fixture's gobo-rotation channel:
    // continuous speed is accumulated over time; indexed mode returns a static angle.
    float goboRotationFor(Fixture *fixture) const;

    // Advances a fixture's motorised attributes (pan/tilt + zoom) toward their DMX
    // targets over time and returns the current smoothed values.
    void updateFixtureMotion(Fixture *fixture, float &panOut, float &tiltOut,
                             float &halfAngleOut) const;

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
