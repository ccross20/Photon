#include "transformcomponent.h"

namespace photon {

TransformComponent::TransformComponent()
{
    m_initialMatrix.setToIdentity();
    m_matrix.setToIdentity();
    m_scale = QVector3D(1.0, 1.0, 1.0);
}
void TransformComponent::setParent(TransformComponent *t_parent)
{
    m_parent = t_parent;
}

TransformComponent *TransformComponent::parent() const
{
    return m_parent;
}

void TransformComponent::rebuildMatrix()
{
    m_matrix = m_initialMatrix;
    m_matrix.translate(m_position);
    m_matrix.scale(m_scale);
    m_matrix.rotate(QQuaternion::fromEulerAngles(m_rotation));

}

void TransformComponent::setPosition(const QVector3D &t_pos)
{
    m_position = t_pos;
    rebuildMatrix();
}

QVector3D TransformComponent::position() const
{
    return m_position;
}

void TransformComponent::setRotationEuler(const QVector3D &t_rotation)
{
    m_rotation = t_rotation;
    rebuildMatrix();
}

void TransformComponent::setScale(const QVector3D &t_scale)
{
    m_scale = t_scale;
    rebuildMatrix();
}

void TransformComponent::setLocalMatrix(const QMatrix4x4 &t_matrix)
{
    m_initialMatrix = t_matrix;
    rebuildMatrix();

}

QVector3D TransformComponent::globalPosition() const
{
    return globalMatrix().map(QVector3D{});
}

QMatrix4x4 TransformComponent::globalMatrix() const
{
    if(m_parent)
        return m_parent->globalMatrix() * m_matrix;
    return m_matrix;
}

QMatrix4x4 TransformComponent::localMatrix() const
{
    return m_matrix;
}


} // namespace photon
