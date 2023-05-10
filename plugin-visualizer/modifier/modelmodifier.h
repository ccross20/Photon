#ifndef PHOTON_MODELMODIFIER_H
#define PHOTON_MODELMODIFIER_H
#include "entity.h"
#include "sceneobjectmodel.h"

namespace photon {

class ModelModifier
{
public:
    ModelModifier(SceneObjectModel *model,  Entity *entity);
    virtual ~ModelModifier();

    Entity* entity() const;
    SceneObjectModel* model() const;
    Fixture *fixture() const;

    virtual void preDraw(const double t_elapsed){}
    virtual void updateModel(const DMXMatrix &t_matrix, const double t_elapsed){}

private:
    SceneObjectModel *m_model;
    Entity *m_entity;

};

} // namespace photon

#endif // PHOTON_MODELMODIFIER_H
