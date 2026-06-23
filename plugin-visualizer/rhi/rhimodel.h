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
//   * a node whose name starts with "pan_"  is the pan joint
//   * a node whose name starts with "tilt_" is the tilt joint
//     (the _x/_y/_z suffix selects the local rotation axis)
//   * a node named "lamp" is the light emitter (beam origin/direction)
//   * a node whose name starts with "lens" is the lens face; it is tinted with the
//     fixture's live emitted colour so the lens glows the current beam colour
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
        bool emitter = false;
        bool lens = false;             // tinted with the live emitted colour
        QVector<Node> children;
    };

    ~RhiModel();

    // Loads a model file (FBX / glTF / OBJ via assimp). Null on failure.
    static RhiModel *load(const QString &path);

    const Node &root() const { return m_root; }
    bool hasEmitter() const { return m_hasEmitter; }
    QVector3D boundsMin() const { return m_min; }
    QVector3D boundsMax() const { return m_max; }

    void upload(QRhi *rhi, QRhiResourceUpdateBatch *batch);
    void releaseGpu();

private:
    Node m_root;
    bool m_hasEmitter = false;
    QVector3D m_min{0, 0, 0};
    QVector3D m_max{0, 0, 0};

    void uploadNode(Node &n, QRhi *rhi, QRhiResourceUpdateBatch *batch);
    void releaseNode(Node &n);
};

} // namespace photon

#endif // PHOTON_RHIMODEL_H
