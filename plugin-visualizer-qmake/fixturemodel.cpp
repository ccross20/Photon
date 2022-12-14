#include "fixturemodel.h"
#include "entity.h"
#include "fixture/fixture.h"
#include "modelloader.h"
#include "component/transformcomponent.h"
#include "modifier/modelmodifier.h"
#include "modifier/panmodelmodifier.h"
#include "modifier/tiltmodelmodifier.h"

namespace photon {




FixtureModel::FixtureModel(Fixture *t_fixture, QObject *parent)
    : QObject{parent},m_fixture(t_fixture)
{
    connect(m_fixture, &Fixture::transformChanged, this, &FixtureModel::transformUpdated);

    ModelLoader loader;
    m_entity = loader.loadResource("C:\\Projects\\photon\\src\\plugin-visualizer\\resources\\model\\moving-head.fbx");

    recursiveAddModifiers(m_entity);

    transformUpdated();
}

void FixtureModel::recursiveAddModifiers(Entity *t_entity)
{
    if(t_entity->name().startsWith("pan_"))
    {
        m_modifiers.append(new PanModelModifier(this, t_entity));
    }

    if(t_entity->name().startsWith("tilt_"))
    {
        m_modifiers.append(new TiltModelModifier(this, t_entity));
    }

    for(auto child : t_entity->children())
    {
        recursiveAddModifiers(child);
    }
}


Entity *FixtureModel::entity() const
{
    return m_entity;
}

Fixture *FixtureModel::fixture() const
{
    return m_fixture;
}

void FixtureModel::transformUpdated()
{
    TransformComponent *xform = m_entity->findComponent<TransformComponent*>();

    xform->setPosition(m_fixture->position());
    xform->setRotationEuler(m_fixture->rotation());
}

void FixtureModel::updateFromDMX(const DMXMatrix &t_matrix)
{

    for(auto modifier : m_modifiers)
    {
        modifier->updateModel(t_matrix);
    }
}

} // namespace photon
