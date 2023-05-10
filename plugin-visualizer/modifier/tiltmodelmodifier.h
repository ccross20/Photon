#ifndef PHOTON_TILTMODELMODIFIER_H
#define PHOTON_TILTMODELMODIFIER_H
#include "modelmodifier.h"

namespace photon {

class TransformComponent;
class TiltCapability;

class Lerper
{
public:
    void setInitial(const QVector3D &value);
    void addValue(const QVector3D &value);
    QVector3D tick(double time);

private:
    QVector3D m_target;
    QVector3D m_acceleration;

};

class TiltModelModifier : public ModelModifier
{


public:
    TiltModelModifier(SceneObjectModel *model,  Entity *entity);


    void preDraw(const double t_elapsed) override;
    void updateModel(const DMXMatrix &t_matrix, const double t_elapsed) override;

private:

    Axis m_axis;
    TransformComponent *m_transform;
    TiltCapability *m_capability;
    Lerper m_lerper;
};

} // namespace photon

#endif // PHOTON_TILTMODELMODIFIER_H
