#include "modelmodifier.h"

namespace photon {

ModelModifier::ModelModifier(FixtureModel *t_model,  Entity *t_entity):m_model(t_model),m_entity(t_entity)
{

}

ModelModifier::~ModelModifier()
{

}

Fixture *ModelModifier::fixture() const
{
    return m_model->fixture();
}

Entity* ModelModifier::entity() const
{
    return m_entity;
}

FixtureModel *ModelModifier::model() const
{
    return m_model;
}

} // namespace photon
