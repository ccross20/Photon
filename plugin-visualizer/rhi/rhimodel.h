#ifndef PHOTON_RHIMODEL_H
#define PHOTON_RHIMODEL_H

#include <QString>
#include <QMatrix4x4>
#include <QColor>
#include <QVector>
#include <QVector3D>

class QRhi;
class QRhiResourceUpdateBatch;

namespace photon {

class RhiMesh;

// A loaded fixture model: the source file's node hierarchy (preserved so pan/tilt
// joints can be animated), one RhiMesh per sub-mesh, plus the resolved rig roles.
//
// Rig convention (in the model, matching the legacy rigs):
//   * a node whose name starts with "pan_"   is the pan joint
//   * a node whose name starts with "tilt_"  is the tilt joint
//   * a node whose name starts with "rotor_" is a third rotation joint that spins its
//     children as a rigid sub-assembly independent of pan/tilt - e.g. a bee-eye's
//     lens plate (the parent of its "lens*" meshes), driven by a fixture-specific
//     rotation channel rather than pan/tilt
//     (all three: the _x/_y/_z suffix selects the local rotation axis)
//   * a node whose name starts with "lamp" is a light emitter (beam origin/direction).
//     A single-emitter model names it exactly "lamp"; a multi-cell model (e.g. a
//     bee-eye) names each one "lamp1".."lampN", authored in the SAME order as the
//     fixture's per-cell channels (index 0 = first cell, etc.)
//   * a node whose name starts with "lens" is a lens face; it is tinted with the
//     fixture's live emitted colour so the lens glows the current beam colour
//   * a node named "origin" is the fixture's reference frame (mount / pan pivot); the
//     model is placed so this node coincides with the fixture's scene transform
class RhiModel
{
public:
    struct Node {
        QString name;
        QMatrix4x4 local;
        QVector<RhiMesh *> meshes;     // owned; one per source aiMesh
        QColor color = QColor(190, 190, 200);
        int  panAxis = -1;             // -1 = none, 0 = X, 1 = Y, 2 = Z
        int  tiltAxis = -1;
        int  rotorAxis = -1;           // third rotation joint, independent of pan/tilt
        bool emitter = false;
        bool lens = false;             // tinted with the live emitted colour
        bool origin = false;           // fixture reference frame (mount / pan pivot)
        QVector<Node> children;
    };

    ~RhiModel();

    // Loads a model file (FBX / glTF / OBJ via assimp). Null on failure.
    static RhiModel *load(const QString &path);

    const Node &root() const { return m_root; }
    bool hasEmitter() const { return m_hasEmitter; }
    // The "origin" null's transform relative to the model root (identity if absent).
    // The renderer places the model by globalMatrix * originTransform().inverted() so
    // the origin coincides with the fixture's scene transform.
    bool hasOrigin() const { return m_hasOrigin; }
    QMatrix4x4 originTransform() const { return m_originTransform; }
    QVector3D boundsMin() const { return m_min; }
    QVector3D boundsMax() const { return m_max; }

    void upload(QRhi *rhi, QRhiResourceUpdateBatch *batch);
    void releaseGpu();

private:
    Node m_root;
    bool m_hasEmitter = false;
    bool m_hasOrigin = false;
    QMatrix4x4 m_originTransform;
    QVector3D m_min{0, 0, 0};
    QVector3D m_max{0, 0, 0};

    void uploadNode(Node &n, QRhi *rhi, QRhiResourceUpdateBatch *batch);
    void releaseNode(Node &n);
};

} // namespace photon

#endif // PHOTON_RHIMODEL_H
