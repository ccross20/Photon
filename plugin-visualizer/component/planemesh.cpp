#include "planemesh.h"

namespace photon {

PlaneMesh::PlaneMesh()
{
    m_size = QVector2D(1, 1);
    setPrimitveType(GL_TRIANGLES);
}

void PlaneMesh::setXSize(double value)
{
    m_size.setX(value);
    setDirty(Dirty_Rebuild);
}

void PlaneMesh::setYSize(double value)
{
    m_size.setY(value);
    setDirty(Dirty_Rebuild);
}

void PlaneMesh::rebuild(QOpenGLContext *context)
{
    QVector<QVector3D> vertices;
    //QVector<GLushort> indices{0,  1,  2,  3, 2, 1,   2,  1,  0,  1, 2, 3 };

    QVector<GLushort> indices{0, 3, 4,
                              0, 4, 1,
                              1, 4, 2,
                              2, 4, 5,
                              5, 4, 8,
                              8, 4, 7,
                              7, 4, 6,
                              6, 4, 3};
    //QVector<GLushort> indices{ 0,  2,  1};

/*
    vertices.append(QVector3D(-1.0f * m_size.x(), 0,  -1.0f * m_size.y()));
    vertices.append(QVector3D( 1.0f * m_size.x(), 0,  -1.0f * m_size.y()));
    vertices.append(QVector3D(-1.0f * m_size.x(), 0,  1.0f * m_size.y()));
    vertices.append(QVector3D( 1.0f * m_size.x(), 0,  1.0f * m_size.y()));
    */

    vertices.append(QVector3D(-1.0f * m_size.x(),   0,  -1.0f * m_size.y()));
    vertices.append(QVector3D(                 0,   0,  -1.0f * m_size.y()));
    vertices.append(QVector3D( 1.0f * m_size.x(),   0,  -1.0f * m_size.y()));
    vertices.append(QVector3D(-1.0f * m_size.x(),   0,                   0));
    vertices.append(QVector3D(                 0,  1.0,                   0));
    vertices.append(QVector3D( 1.0f * m_size.x(),   0,                   0));
    vertices.append(QVector3D(-1.0f * m_size.x(),   0,   1.0f * m_size.y()));
    vertices.append(QVector3D(                 0,   0,   1.0f * m_size.y()));
    vertices.append(QVector3D( 1.0f * m_size.x(),   0,   1.0f * m_size.y()));



    QVector<QVector2D> uvs;
    /*
    uvs.append(QVector2D(0, 1.0));
    uvs.append(QVector2D(1.0, 1.0));
    uvs.append(QVector2D(0, 0));
    uvs.append(QVector2D(1.0, 0));
    */

    uvs.append(QVector2D(  0,   0));
    uvs.append(QVector2D( .5,   0));
    uvs.append(QVector2D(  1,   0));
    uvs.append(QVector2D(  0,  .5));
    uvs.append(QVector2D( .5,  .5));
    uvs.append(QVector2D(  1,  .5));
    uvs.append(QVector2D(  0,   1));
    uvs.append(QVector2D( .5,   1));
    uvs.append(QVector2D(  1,   1));


    setVertices(vertices);
    setUVs(uvs);
    setIndices(indices);

    AbstractMesh::rebuild(context);
}

} // namespace photon
