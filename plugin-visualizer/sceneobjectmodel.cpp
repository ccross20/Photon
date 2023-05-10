#include "sceneobjectmodel.h"
#include "entity.h"
#include "scene/sceneobject.h"
#include "component/transformcomponent.h"
#include "modelfactory.h"

namespace photon {




SceneObjectModel::SceneObjectModel(SceneObject *t_sceneObj, QObject *parent)
    : QObject{parent},m_sceneObj(t_sceneObj)
{
    connect(t_sceneObj, &SceneObject::matrixChanged, this, &SceneObjectModel::transformUpdated);
    connect(t_sceneObj, &SceneObject::metadataChanged, this, &SceneObjectModel::metadataChanged);
    connect(t_sceneObj, &SceneObject::childWasAdded, this, &SceneObjectModel::childWasAdded);
    connect(t_sceneObj, &SceneObject::childWasRemoved, this, &SceneObjectModel::childWasRemoved);

    Entity *e = new Entity;

    e->addComponent(new TransformComponent);

    setEntity(e);


}

SceneObjectModel::~SceneObjectModel()
{

}

void SceneObjectModel::setEntity(Entity *t_entity)
{
    if(m_entity)
        delete m_entity;

    m_entity = t_entity;
    transformUpdated();

    TransformComponent *xform = m_entity->findComponent<TransformComponent*>();

    for(auto child : m_sceneObj->sceneChildren())
    {
        auto childModel = ModelFactory::createModel(child);
        childModel->entity()->setParent(m_entity);

        TransformComponent *childXForm = childModel->entity()->findComponent<TransformComponent*>();
        childXForm->setParent(xform);
        m_children.append(childModel);
    }
}

Entity *SceneObjectModel::entity() const
{
    return m_entity;
}

SceneObject *SceneObjectModel::sceneObject() const
{
    return m_sceneObj;
}

void SceneObjectModel::metadataChanged()
{
    metadataUpdated();
}

void SceneObjectModel::metadataUpdated()
{

}

void SceneObjectModel::transformUpdated()
{
    TransformComponent *xform = m_entity->findComponent<TransformComponent*>();

    xform->setPosition(m_sceneObj->position());
    xform->setRotationEuler(m_sceneObj->rotation());
}

void SceneObjectModel::childWasAdded(photon::SceneObject *t_child)
{

    TransformComponent *xform = m_entity->findComponent<TransformComponent*>();
    auto childModel = ModelFactory::createModel(t_child);
    childModel->entity()->setParent(m_entity);

    TransformComponent *childXForm = childModel->entity()->findComponent<TransformComponent*>();
    childXForm->setParent(xform);

    m_children.append(childModel);

}

void SceneObjectModel::childWasRemoved(photon::SceneObject *t_child)
{
    for(auto child : m_children)
    {
        if(child->sceneObject() == t_child)
        {
            m_children.removeOne(child);
            child->entity()->destroy();
            delete child;
        }
    }
}

void SceneObjectModel::preDraw(double t_elapsed)
{
    for(auto child : m_children)
    {
        child->preDraw(t_elapsed);
    }
}

void SceneObjectModel::updateFromDMX(const DMXMatrix &t_matrix, const double t_elapsed)
{
    for(auto child : m_children)
    {
        child->updateFromDMX(t_matrix, t_elapsed);
    }

}

} // namespace photon
