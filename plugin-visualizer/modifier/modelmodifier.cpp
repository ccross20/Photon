#include "modelmodifier.h"
#include "fixture/fixture.h"

namespace photon {

ModelModifier::ModelModifier(SceneObjectModel *t_model,  Entity *t_entity):m_model(t_model),m_entity(t_entity)
{

}

ModelModifier::~ModelModifier()
{

}

Fixture *ModelModifier::fixture() const
{
    return static_cast<Fixture*>(m_model->sceneObject());
}

Entity* ModelModifier::entity() const
{
    return m_entity;
}

SceneObjectModel *ModelModifier::model() const
{
    return m_model;
}

} // namespace photon
