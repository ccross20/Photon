#ifndef PHOTON_TRANSFORMCOMPONENT_H
#define PHOTON_TRANSFORMCOMPONENT_H

#include "abstractcomponent.h"

namespace photon {

class TransformComponent : public AbstractComponent
{
public:
    TransformComponent();

    void setParent(TransformComponent *parent);
    TransformComponent *parent() const;

    void setPosition(const QVector3D &pos);
    QVector3D position() const;

    void setRotationEuler(const QVector3D &rotation);
    void setScale(const QVector3D &scale);
    void setLocalMatrix(const QMatrix4x4 &matrix);

    QVector3D globalPosition() const;
    QMatrix4x4 globalMatrix() const;
    QMatrix4x4 localMatrix() const;

private:
    void rebuildMatrix();

    QVector3D m_position;
    QVector3D m_rotation;
    QVector3D m_scale;
    QMatrix4x4 m_matrix;
    QMatrix4x4 m_initialMatrix;
    TransformComponent *m_parent = nullptr;
};

} // namespace photon

#endif // PHOTON_TRANSFORMCOMPONENT_H
