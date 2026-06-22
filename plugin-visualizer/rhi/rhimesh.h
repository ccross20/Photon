#ifndef PHOTON_RHIMESH_H
#define PHOTON_RHIMESH_H

#include <QByteArray>

class QRhi;
class QRhiBuffer;
class QRhiResourceUpdateBatch;

namespace photon {

// A GPU mesh: holds CPU-side vertex/index data and the QRhi buffers built from
// it. The CPU data is retained so buffers can be rebuilt after a device reset.
//
// Vertex layout is not described here — the pipeline that draws the mesh owns
// the QRhiVertexInputLayout. Two layouts are used by Milestone A:
//   * lit mesh  : vec3 position, vec3 normal   (createBox)
//   * line list : vec3 position, vec3 color    (createGrid)
class RhiMesh
{
public:
    RhiMesh();
    ~RhiMesh();

    void setVertexData(const QByteArray &data, int vertexCount);
    void setIndexData(const QByteArray &data, int indexCount);

    // Creates the QRhiBuffers (if needed) and queues uploads on the batch.
    void upload(QRhi *rhi, QRhiResourceUpdateBatch *batch);
    void release();

    QRhiBuffer *vertexBuffer() const { return m_vertexBuffer; }
    QRhiBuffer *indexBuffer() const { return m_indexBuffer; }
    int vertexCount() const { return m_vertexCount; }
    int indexCount() const { return m_indexCount; }
    bool isIndexed() const { return m_indexCount > 0; }

    // Geometry generators.
    static RhiMesh *createBox(float halfX, float halfY, float halfZ);
    static RhiMesh *createGrid(int halfExtent, float spacing);
    // Unit quad in the local XY plane (corners ±0.5), normal +Z. Scale via the
    // model matrix. Used for wall/floor surfaces (lit, position + normal layout).
    static RhiMesh *createPlane();
    // Box-truss: `beams` chord tubes on a circle of radius `offset`, running along
    // the local X axis over `length`, with diagonal lacing every `segmentLength`.
    static RhiMesh *createTruss(int beams, float length, float offset,
                                float radius, float segmentLength);
    // A hollow light-beam cone: apex at the origin, axis along -Y, unit length and
    // unit base radius (scale via the model matrix). The second vertex attribute
    // carries a per-vertex alpha factor (bright apex → faint base) for additive
    // blending rather than a normal.
    static RhiMesh *createCone(int sides);

private:
    QByteArray   m_vertexData;
    QByteArray   m_indexData;
    int          m_vertexCount = 0;
    int          m_indexCount = 0;
    QRhiBuffer  *m_vertexBuffer = nullptr;
    QRhiBuffer  *m_indexBuffer = nullptr;
    bool         m_uploaded = false;
};

} // namespace photon

#endif // PHOTON_RHIMESH_H
