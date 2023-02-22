#ifndef PHOTON_ABSTRACTMESH_H
#define PHOTON_ABSTRACTMESH_H

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include "component/abstractcomponent.h"

namespace photon {

class AbstractMesh : public AbstractComponent
{
public:
    AbstractMesh();
    virtual ~AbstractMesh(){}

    virtual void create(QOpenGLContext *context) override;
    virtual void rebuild(QOpenGLContext *context) override;
    virtual void drawMesh(DrawContext *context, QOpenGLShaderProgram *program);
    virtual void destroy(QOpenGLContext *context) override;
    virtual void draw(DrawContext *context) override;

    void setPrimitveType(int value);

    void clearVertices();
    void setVertices(const QVector<QVector3D> &);
    void clearNormals();
    void setNormals(const QVector<QVector3D> &);
    void clearUVs();
    void setUVs(const QVector<QVector2D> &);
    void clearIndices();
    void setIndices(const QVector<GLushort> &);

private:
    QVector<QVector3D> m_vertices;
    QVector<QVector3D> m_tempVertices;
    QVector<QVector3D> m_normals;
    QVector<QVector3D> m_tempNormals;
    QVector<QVector2D> m_uvs;
    QVector<QVector2D> m_tempUvs;
    QVector<GLushort> m_indices;
    QVector<GLushort> m_tempIndices;

    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_uvsBuffer;
    QOpenGLBuffer m_normalBuffer;
    QOpenGLBuffer m_indexBuffer;

    //int m_primitve = GL_TRIANGLE_STRIP;
    int m_primitve = GL_TRIANGLES;
};

} // namespace photon

#endif // PHOTON_ABSTRACTMESH_H
