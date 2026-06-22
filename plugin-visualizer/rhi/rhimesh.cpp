#include <rhi/qrhi.h>
#include <QVector3D>
#include "rhimesh.h"

namespace photon {

RhiMesh::RhiMesh() {}

RhiMesh::~RhiMesh()
{
    release();
}

void RhiMesh::setVertexData(const QByteArray &data, int vertexCount)
{
    m_vertexData = data;
    m_vertexCount = vertexCount;
    m_uploaded = false;
}

void RhiMesh::setIndexData(const QByteArray &data, int indexCount)
{
    m_indexData = data;
    m_indexCount = indexCount;
    m_uploaded = false;
}

void RhiMesh::upload(QRhi *rhi, QRhiResourceUpdateBatch *batch)
{
    if (m_uploaded)
        return;

    if (!m_vertexBuffer) {
        m_vertexBuffer = rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer,
                                        m_vertexData.size());
        m_vertexBuffer->create();
    }
    batch->uploadStaticBuffer(m_vertexBuffer, m_vertexData.constData());

    if (m_indexCount > 0) {
        if (!m_indexBuffer) {
            m_indexBuffer = rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::IndexBuffer,
                                           m_indexData.size());
            m_indexBuffer->create();
        }
        batch->uploadStaticBuffer(m_indexBuffer, m_indexData.constData());
    }

    m_uploaded = true;
}

void RhiMesh::release()
{
    delete m_vertexBuffer;
    m_vertexBuffer = nullptr;
    delete m_indexBuffer;
    m_indexBuffer = nullptr;
    m_uploaded = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Geometry generators
// ─────────────────────────────────────────────────────────────────────────────

namespace {

void pushVec3(QByteArray &buf, const QVector3D &v)
{
    const float f[3] = { v.x(), v.y(), v.z() };
    buf.append(reinterpret_cast<const char *>(f), sizeof(f));
}

// Appends a quad (two triangles) of lit vertices (position + normal).
void pushQuad(QByteArray &verts, QByteArray &indices, int &base,
              const QVector3D &a, const QVector3D &b,
              const QVector3D &c, const QVector3D &d,
              const QVector3D &normal)
{
    pushVec3(verts, a); pushVec3(verts, normal);
    pushVec3(verts, b); pushVec3(verts, normal);
    pushVec3(verts, c); pushVec3(verts, normal);
    pushVec3(verts, d); pushVec3(verts, normal);

    const quint16 idx[6] = {
        quint16(base + 0), quint16(base + 1), quint16(base + 2),
        quint16(base + 0), quint16(base + 2), quint16(base + 3)
    };
    indices.append(reinterpret_cast<const char *>(idx), sizeof(idx));
    base += 4;
}

void pushLine(QByteArray &verts, const QVector3D &a, const QVector3D &b, const QVector3D &color)
{
    pushVec3(verts, a); pushVec3(verts, color);
    pushVec3(verts, b); pushVec3(verts, color);
}

constexpr float kPi = 3.14159265358979323846f;

// Appends an open cylinder (tube) between p0 and p1 with lit vertices (pos+normal).
void addCylinder(QByteArray &verts, QByteArray &indices, int &base,
                 const QVector3D &p0, const QVector3D &p1, float radius, int sides)
{
    QVector3D axis = p1 - p0;
    const float len = axis.length();
    if (len < 1e-6f)
        return;
    axis /= len;

    const QVector3D ref = std::abs(axis.y()) > 0.99f ? QVector3D(1, 0, 0) : QVector3D(0, 1, 0);
    const QVector3D right = QVector3D::crossProduct(ref, axis).normalized();
    const QVector3D up = QVector3D::crossProduct(axis, right);

    const int ringBase = base;
    for (int s = 0; s <= sides; ++s) {
        const float a = (2.0f * kPi) * float(s) / float(sides);
        const QVector3D dir = std::cos(a) * right + std::sin(a) * up;
        pushVec3(verts, p0 + dir * radius); pushVec3(verts, dir);
        pushVec3(verts, p1 + dir * radius); pushVec3(verts, dir);
    }
    base += (sides + 1) * 2;

    for (int s = 0; s < sides; ++s) {
        const quint16 i0 = quint16(ringBase + s * 2);       // p0 @ s
        const quint16 i1 = quint16(ringBase + s * 2 + 1);   // p1 @ s
        const quint16 i2 = quint16(ringBase + (s + 1) * 2); // p0 @ s+1
        const quint16 i3 = quint16(ringBase + (s + 1) * 2 + 1); // p1 @ s+1
        const quint16 idx[6] = { i0, i1, i3, i0, i3, i2 };
        indices.append(reinterpret_cast<const char *>(idx), sizeof(idx));
    }
}

} // namespace

RhiMesh *RhiMesh::createBox(float hx, float hy, float hz)
{
    QByteArray verts;
    QByteArray indices;
    int base = 0;

    // Corner naming: signs of (x, y, z)
    const QVector3D ppp( hx,  hy,  hz), ppn( hx,  hy, -hz);
    const QVector3D pnp( hx, -hy,  hz), pnn( hx, -hy, -hz);
    const QVector3D npp(-hx,  hy,  hz), npn(-hx,  hy, -hz);
    const QVector3D nnp(-hx, -hy,  hz), nnn(-hx, -hy, -hz);

    pushQuad(verts, indices, base, pnp, ppp, ppn, pnn, QVector3D( 1, 0, 0)); // +X
    pushQuad(verts, indices, base, nnn, npn, npp, nnp, QVector3D(-1, 0, 0)); // -X
    pushQuad(verts, indices, base, npn, ppn, ppp, npp, QVector3D( 0, 1, 0)); // +Y
    pushQuad(verts, indices, base, nnn, nnp, pnp, pnn, QVector3D( 0,-1, 0)); // -Y
    pushQuad(verts, indices, base, nnp, npp, ppp, pnp, QVector3D( 0, 0, 1)); // +Z
    pushQuad(verts, indices, base, pnn, ppn, npn, nnn, QVector3D( 0, 0,-1)); // -Z

    auto *mesh = new RhiMesh;
    mesh->setVertexData(verts, 24);
    mesh->setIndexData(indices, 36);
    return mesh;
}

RhiMesh *RhiMesh::createGrid(int halfExtent, float spacing)
{
    QByteArray verts;
    int count = 0;

    const QVector3D minor(0.30f, 0.30f, 0.30f);
    const QVector3D xAxis(0.65f, 0.25f, 0.25f);
    const QVector3D zAxis(0.25f, 0.35f, 0.65f);

    const float extent = halfExtent * spacing;

    for (int i = -halfExtent; i <= halfExtent; ++i) {
        const float p = i * spacing;

        // Lines parallel to the Z axis (vary X). The x==0 line marks the Z axis.
        pushLine(verts, QVector3D(p, 0.0f, -extent), QVector3D(p, 0.0f, extent),
                 i == 0 ? zAxis : minor);
        count += 2;

        // Lines parallel to the X axis (vary Z). The z==0 line marks the X axis.
        pushLine(verts, QVector3D(-extent, 0.0f, p), QVector3D(extent, 0.0f, p),
                 i == 0 ? xAxis : minor);
        count += 2;
    }

    auto *mesh = new RhiMesh;
    mesh->setVertexData(verts, count);
    return mesh;
}

RhiMesh *RhiMesh::createTruss(int beams, float length, float offset,
                              float radius, float segmentLength)
{
    beams = qMax(2, beams);
    length = qMax(0.01f, length);
    segmentLength = qMax(0.01f, segmentLength);

    QByteArray verts;
    QByteArray indices;
    int base = 0;

    const float halfL = length * 0.5f;

    // Chord cross-section offsets in the local YZ plane.
    QVector<QVector3D> chord(beams);
    for (int i = 0; i < beams; ++i) {
        const float a = (2.0f * kPi) * float(i) / float(beams);
        chord[i] = QVector3D(0.0f, std::cos(a) * offset, std::sin(a) * offset);
    }

    // Longitudinal chord tubes along X.
    for (int i = 0; i < beams; ++i) {
        const QVector3D p0(-halfL, chord[i].y(), chord[i].z());
        const QVector3D p1( halfL, chord[i].y(), chord[i].z());
        addCylinder(verts, indices, base, p0, p1, radius, 8);
    }

    // Diagonal lacing between adjacent chords, capped to keep 16-bit indices valid.
    const int segs = qBound(1, int(std::round(length / segmentLength)), 200);
    for (int i = 0; i < beams; ++i) {
        const int j = (i + 1) % beams;
        bool zig = true;
        for (int s = 0; s < segs; ++s) {
            const float x0 = -halfL + length * float(s) / segs;
            const float x1 = -halfL + length * float(s + 1) / segs;
            const QVector3D a = zig ? QVector3D(x0, chord[i].y(), chord[i].z())
                                    : QVector3D(x0, chord[j].y(), chord[j].z());
            const QVector3D b = zig ? QVector3D(x1, chord[j].y(), chord[j].z())
                                    : QVector3D(x1, chord[i].y(), chord[i].z());
            addCylinder(verts, indices, base, a, b, radius * 0.5f, 6);
            zig = !zig;
        }
    }

    auto *mesh = new RhiMesh;
    mesh->setVertexData(verts, base);
    mesh->setIndexData(indices, int(indices.size() / sizeof(quint16)));
    return mesh;
}

RhiMesh *RhiMesh::createPlane()
{
    QByteArray verts;
    QByteArray indices;
    int base = 0;

    const QVector3D n(0.0f, 0.0f, 1.0f);
    const QVector3D a(-0.5f, -0.5f, 0.0f);
    const QVector3D b( 0.5f, -0.5f, 0.0f);
    const QVector3D c( 0.5f,  0.5f, 0.0f);
    const QVector3D d(-0.5f,  0.5f, 0.0f);
    pushQuad(verts, indices, base, a, b, c, d, n);

    auto *mesh = new RhiMesh;
    mesh->setVertexData(verts, 4);
    mesh->setIndexData(indices, 6);
    return mesh;
}

RhiMesh *RhiMesh::createCone(int sides)
{
    sides = qMax(3, sides);

    QByteArray verts;
    QByteArray indices;

    // Second attribute holds (alpha, 0, 0) — bright at the apex, faint at the base.
    auto pushV = [&](const QVector3D &p, float alpha) {
        const float f[6] = { p.x(), p.y(), p.z(), alpha, 0.0f, 0.0f };
        verts.append(reinterpret_cast<const char *>(f), sizeof(f));
    };

    constexpr float apexAlpha = 0.85f;
    constexpr float baseAlpha = 0.04f;

    pushV(QVector3D(0, 0, 0), apexAlpha);  // vertex 0 = apex
    for (int s = 0; s < sides; ++s) {
        const float a = (2.0f * kPi) * float(s) / float(sides);
        pushV(QVector3D(std::cos(a), -1.0f, std::sin(a)), baseAlpha);
    }

    for (int s = 0; s < sides; ++s) {
        const quint16 r0 = quint16(1 + s);
        const quint16 r1 = quint16(1 + (s + 1) % sides);
        const quint16 idx[3] = { 0, r0, r1 };
        indices.append(reinterpret_cast<const char *>(idx), sizeof(idx));
    }

    auto *mesh = new RhiMesh;
    mesh->setVertexData(verts, sides + 1);
    mesh->setIndexData(indices, sides * 3);
    return mesh;
}

} // namespace photon
