#ifndef PHOTON_MODELMODIFIER_H
#define PHOTON_MODELMODIFIER_H
#include "entity.h"
#include "fixturemodel.h"

namespace photon {

class ModelModifier
{
public:
    ModelModifier(FixtureModel *model,  Entity *entity);
    virtual ~ModelModifier();

    Entity* entity() const;
    FixtureModel* model() const;
    Fixture *fixture() const;

    virtual void updateModel(const DMXMatrix &t_matrix){}

private:
    FixtureModel *m_model;
    Entity *m_entity;

};

} // namespace photon

#endif // PHOTON_MODELMODIFIER_H
