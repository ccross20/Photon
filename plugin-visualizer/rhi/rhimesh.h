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
    // Box-truss: `beams` chord tubes on a circle of radius `offset`, running along
    // the local X axis over `length`, with diagonal lacing every `segmentLength`.
    static RhiMesh *createTruss(int beams, float length, float offset,
                                float radius, float segmentLength);

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
