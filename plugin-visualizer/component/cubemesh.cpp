#include "cubemesh.h"

namespace photon {

CubeMesh::CubeMesh()
{
    m_size = QVector3D(1, 1, 1);
}

void CubeMesh::setXSize(double value)
{
    m_size.setX(value);
    setDirty(Dirty_Rebuild);
}

void CubeMesh::setYSize(double value)
{
    m_size.setY(value);
    setDirty(Dirty_Rebuild);
}

void CubeMesh::setZSize(double value)
{
    m_size.setZ(value);
    setDirty(Dirty_Rebuild);
}

void CubeMesh::rebuild(QOpenGLContext *context)
{
    qDebug() << "Rebuild";
    QVector<QVector3D> vertices;
    QVector<GLushort> indices{0,  1,  2,  3,  3,     // Face 0 - triangle strip ( v0,  v1,  v2,  v3)
                           4,  4,  5,  6,  7,  7, // Face 1 - triangle strip ( v4,  v5,  v6,  v7)
                           8,  8,  9, 10, 11, 11, // Face 2 - triangle strip ( v8,  v9, v10, v11)
                          12, 12, 13, 14, 15, 15, // Face 3 - triangle strip (v12, v13, v14, v15)
                          16, 16, 17, 18, 19, 19, // Face 4 - triangle strip (v16, v17, v18, v19)
                          20, 20, 21, 22, 23   };

    vertices.append(QVector3D(-1.0f * m_size.x(), -1.0f * m_size.y(),  1.0f * m_size.z()));
    vertices.append(QVector3D( 1.0f * m_size.x(), -1.0f * m_size.y(),  1.0f * m_size.z()));
    vertices.append(QVector3D(-1.0f * m_size.x(),  1.0f * m_size.y(),  1.0f * m_size.z()));
    vertices.append(QVector3D( 1.0f * m_size.x(),  1.0f * m_size.y(),  1.0f * m_size.z()));

    vertices.append(QVector3D( 1.0f * m_size.x(), -1.0f * m_size.y(),  1.0f * m_size.z()));
    vertices.append(QVector3D( 1.0f * m_size.x(), -1.0f * m_size.y(), -1.0f * m_size.z()));
    vertices.append(QVector3D( 1.0f * m_size.x(),  1.0f * m_size.y(),  1.0f * m_size.z()));
    vertices.append(QVector3D( 1.0f * m_size.x(),  1.0f * m_size.y(), -1.0f * m_size.z()));

    vertices.append(QVector3D( 1.0f * m_size.x(), -1.0f * m_size.y(), -1.0f * m_size.z()));
    vertices.append(QVector3D(-1.0f * m_size.x(), -1.0f * m_size.y(), -1.0f * m_size.z()));
    vertices.append(QVector3D( 1.0f * m_size.x(),  1.0f * m_size.y(), -1.0f * m_size.z()));
    vertices.append(QVector3D(-1.0f * m_size.x(),  1.0f * m_size.y(), -1.0f * m_size.z()));

    vertices.append(QVector3D(-1.0f * m_size.x(), -1.0f * m_size.y(), -1.0f * m_size.z()));
    vertices.append(QVector3D(-1.0f * m_size.x(), -1.0f * m_size.y(),  1.0f * m_size.z()));
    vertices.append(QVector3D(-1.0f * m_size.x(),  1.0f * m_size.y(), -1.0f * m_size.z()));
    vertices.append(QVector3D(-1.0f * m_size.x(),  1.0f * m_size.y(),  1.0f * m_size.z()));

    vertices.append(QVector3D(-1.0f * m_size.x(), -1.0f * m_size.y(), -1.0f * m_size.z()));
    vertices.append(QVector3D( 1.0f * m_size.x(), -1.0f * m_size.y(), -1.0f * m_size.z()));
    vertices.append(QVector3D(-1.0f * m_size.x(), -1.0f * m_size.y(),  1.0f * m_size.z()));
    vertices.append(QVector3D( 1.0f * m_size.x(), -1.0f * m_size.y(),  1.0f * m_size.z()));

    vertices.append(QVector3D(-1.0f * m_size.x(),  1.0f * m_size.y(),  1.0f * m_size.z()));
    vertices.append(QVector3D( 1.0f * m_size.x(),  1.0f * m_size.y(),  1.0f * m_size.z()));
    vertices.append(QVector3D(-1.0f * m_size.x(),  1.0f * m_size.y(), -1.0f * m_size.z()));
    vertices.append(QVector3D( 1.0f * m_size.x(),  1.0f * m_size.y(), -1.0f * m_size.z()));


    QVector<QVector3D> normals;
    normals.append(QVector3D(0, 0, 1.0f));
    normals.append(QVector3D(0, 0, 1.0f));
    normals.append(QVector3D(0, 0, 1.0f));
    normals.append(QVector3D(0, 0, 1.0f));

    normals.append(QVector3D( 1.0f, 0, 0));
    normals.append(QVector3D( 1.0f, 0, 0));
    normals.append(QVector3D( 1.0f, 0, 0));
    normals.append(QVector3D( 1.0f, 0, 0));

    normals.append(QVector3D( 0, 0, -1.0f));
    normals.append(QVector3D( 0, 0, -1.0f));
    normals.append(QVector3D( 0, 0, -1.0f));
    normals.append(QVector3D( 0, 0, -1.0f));

    normals.append(QVector3D(-1.0f, 0, 0));
    normals.append(QVector3D(-1.0f, 0, 0));
    normals.append(QVector3D(-1.0f, 0, 0));
    normals.append(QVector3D(-1.0f, 0, 0));

    normals.append(QVector3D(0, -1.0f, 0));
    normals.append(QVector3D(0, -1.0f, 0));
    normals.append(QVector3D(0, -1.0f, 0));
    normals.append(QVector3D(0, -1.0f, 0));

    normals.append(QVector3D(0,  1.0f, 0));
    normals.append(QVector3D(0,  1.0f, 0));
    normals.append(QVector3D(0,  1.0f, 0));
    normals.append(QVector3D(0,  1.0f, 0));

    setVertices(vertices);
    setNormals(normals);
    setIndices(indices);

    AbstractMesh::rebuild(context);
}

} // namespace photon
