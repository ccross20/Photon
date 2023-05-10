#include <QOpenGLExtraFunctions>
#include "abstractmesh.h"
#include "entity.h"

namespace photon {

AbstractMesh::AbstractMesh(): AbstractComponent(),m_indexBuffer(QOpenGLBuffer::IndexBuffer)
{

}

void AbstractMesh::create(QOpenGLContext *context)
{
    m_vertexBuffer.create();
    m_uvsBuffer.create();
    m_normalBuffer.create();
    m_indexBuffer.create();

    AbstractComponent::create(context);
}

void AbstractMesh::rebuild(QOpenGLContext *context)
{
    m_vertices = m_tempVertices;
    m_indices = m_tempIndices;
    m_normals = m_tempNormals;
    m_uvs = m_tempUvs;


    m_vertexBuffer.bind();
    m_vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertexBuffer.allocate(m_vertices.data(), m_vertices.size() * sizeof(QVector3D));

    m_normalBuffer.bind();
    m_normalBuffer.allocate(m_normals.data(), m_normals.size() * sizeof(QVector3D));

    m_uvsBuffer.bind();
    m_uvsBuffer.allocate(m_uvs.data(), m_uvs.size() * sizeof(QVector2D));

    // Transfer index data to VBO 1
    m_indexBuffer.bind();
    m_indexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_indexBuffer.allocate(m_indices.data(), m_indices.size() * sizeof(GLushort));



    AbstractComponent::rebuild(context);
}

void AbstractMesh::draw(DrawContext *context)
{
    AbstractComponent::draw(context);
}

void AbstractMesh::drawMesh(DrawContext *context, QOpenGLShaderProgram *program)
{

    if(dirty() & Dirty_Rebuild)
        rebuild(context->openGLContext);
    m_vertexBuffer.bind();
    m_indexBuffer.bind();
    int vertexLocation = program->attributeLocation("aPos");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, sizeof(QVector3D));
/*
    m_uvsBuffer.bind();
    int uvLocation = program->attributeLocation("aUV");
    if(uvLocation >= 0)
    {
        program->enableAttributeArray(uvLocation);
        program->setAttributeBuffer(uvLocation, GL_FLOAT, 0, 2, sizeof(QVector2D));
    }
    */

    m_normalBuffer.bind();
    int normalLocation = program->attributeLocation("aNormal");
    if(normalLocation >= 0 && !m_normals.isEmpty())
    {
        program->enableAttributeArray(normalLocation);
        program->setAttributeBuffer(normalLocation, GL_FLOAT, 0, 3, sizeof(QVector3D));
    }



    //if(entities().length() > 0)
    //    qDebug() << "Draw: "<< entities().front()->name() << "   " << m_vertices.size();


    glDrawElements(m_primitve, m_indices.size(), GL_UNSIGNED_SHORT, nullptr);

    AbstractComponent::draw(context);
}

void AbstractMesh::destroy(QOpenGLContext *context)
{
    m_vertexBuffer.destroy();
    m_normalBuffer.destroy();
    m_uvsBuffer.destroy();
    m_indexBuffer.destroy();
    AbstractComponent::destroy(context);
}

void AbstractMesh::setPrimitveType(int value)
{
    m_primitve = value;
}

void AbstractMesh::clearVertices()
{
    m_tempVertices.clear();
    setDirty(Dirty_Rebuild);
}

void AbstractMesh::clearNormals()
{
    m_tempNormals.clear();
    setDirty(Dirty_Rebuild);
}

void AbstractMesh::clearUVs()
{
    m_tempUvs.clear();
    setDirty(Dirty_Rebuild);
}

void AbstractMesh::setUVs(const QVector<QVector2D> &t_uvs)
{
    m_tempUvs = t_uvs;
    setDirty(Dirty_Rebuild);
}

void AbstractMesh::setVertices(const QVector<QVector3D> &t_vertices)
{
    m_tempVertices = t_vertices;
    setDirty(Dirty_Rebuild);
}

void AbstractMesh::setNormals(const QVector<QVector3D> &t_normals)
{
    m_tempNormals = t_normals;
    setDirty(Dirty_Rebuild);
}

void AbstractMesh::clearIndices()
{
    m_tempIndices.clear();
    setDirty(Dirty_Rebuild);
}

void AbstractMesh::setIndices(const QVector<GLushort> &t_indices)
{
    m_tempIndices = t_indices;
    setDirty(Dirty_Rebuild);
}


} // namespace photon
