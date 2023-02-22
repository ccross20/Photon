#include "entity.h"
#include "component/abstractcomponent.h"

namespace photon {

Entity::Entity(Entity *t_parent):m_parent(t_parent)
{
    if(m_parent)
        m_parent->addChild(this);
}

Entity *Entity::parent() const
{
    return m_parent;
}

Entity *Entity::root() const
{
    if(m_parent)
        return m_parent;
    return const_cast<Entity*>(this);
}

void Entity::setParent(Entity *t_parent)
{
    if(m_parent)
        m_parent->removeChild(this);
    m_parent = t_parent;
    if(m_parent)
        m_parent->addChild(this);
}

void Entity::gatherFamily(QVector<Entity*> &t_results) const
{
    t_results.append(const_cast<Entity*>(this));
    for(auto childIt = m_children.begin(); childIt != m_children.end(); ++childIt)
    {
        (*childIt)->gatherFamily(t_results);
    }
}

void Entity::addChild(Entity *t_child)
{
    if(m_children.contains(t_child))
        return;
    m_children.append(t_child);

    connect(t_child, &Entity::componentAdded, this, &Entity::componentAddedToDescendant);
    connect(t_child, &Entity::componentRemoved, this, &Entity::componentRemovedFromDescendant);
    connect(t_child, &Entity::componentAddedToDescendant, this, &Entity::componentAddedToDescendant);
    connect(t_child, &Entity::componentRemovedFromDescendant, this, &Entity::componentRemovedFromDescendant);

    connect(t_child, &Entity::descendantAdded, this, &Entity::descendantAdded);
    connect(t_child, &Entity::descendantRemoved, this, &Entity::descendantRemoved);
    emit descendantAdded(t_child);
}

void Entity::removeChild(Entity *t_child)
{
    if(!m_children.contains(t_child))
        return;
    m_children.removeOne(t_child);

    disconnect(t_child, &Entity::componentAdded, this, &Entity::componentAddedToDescendant);
    disconnect(t_child, &Entity::componentRemoved, this, &Entity::componentRemovedFromDescendant);
    disconnect(t_child, &Entity::componentAddedToDescendant, this, &Entity::componentAddedToDescendant);
    disconnect(t_child, &Entity::componentRemovedFromDescendant, this, &Entity::componentRemovedFromDescendant);

    disconnect(t_child, &Entity::descendantAdded, this, &Entity::descendantAdded);
    disconnect(t_child, &Entity::descendantRemoved, this, &Entity::descendantRemoved);
    emit descendantRemoved(t_child);
}

void Entity::addComponent(AbstractComponent *t_component)
{
    if(m_components.contains(t_component))
        return;
    m_components.append(t_component);
    t_component->addEntity(this);

    auto xform = dynamic_cast<TransformComponent*>(t_component);
    if(xform)
        m_transform = xform;

    emit componentAdded(t_component);
}

void Entity::removeComponent(AbstractComponent *t_component)
{
    if(!m_components.contains(t_component))
        return;
    m_components.removeOne(t_component);
    t_component->removeEntity(this);

    auto xform = dynamic_cast<TransformComponent*>(t_component);
    if(xform && m_transform == xform)
        m_transform = nullptr;

    emit componentRemoved(t_component);
}

void Entity::setDirty(int dirty)
{
    m_dirty |= dirty;
}

int Entity::dirty() const
{
    return m_dirty;
}

QString Entity::name() const
{
    return m_name;
}

void Entity::setName(const QString &t_name)
{
    m_name = t_name;
}

void Entity::create(QOpenGLContext *context)
{
    m_dirty ^= Dirty_Init;
    for(auto it = m_components.cbegin(); it != m_components.cend(); ++it)
    {
        if((*it)->dirty() & Dirty_Init)
            (*it)->create(context);
    }

    for(auto childIt = m_children.begin(); childIt != m_children.end(); ++childIt)
    {
        (*childIt)->create(context);
    }
}

void Entity::rebuild(QOpenGLContext *context)
{
    m_dirty ^= Dirty_Rebuild;
    for(auto it = m_components.cbegin(); it != m_components.cend(); ++it)
    {
        if((*it)->dirty() & Dirty_Rebuild)
            (*it)->rebuild(context);
    }

    for(auto childIt = m_children.begin(); childIt != m_children.end(); ++childIt)
    {
        (*childIt)->rebuild(context);
    }
}

void Entity::draw(DrawContext *context)
{
    m_dirty ^= Dirty_Draw;
    for(auto it = m_components.cbegin(); it != m_components.cend(); ++it)
    {
        if((*it)->dirty() & Dirty_Draw)
            (*it)->draw(context);
    }

    for(auto childIt = m_children.begin(); childIt != m_children.end(); ++childIt)
    {
        (*childIt)->draw(context);
    }
}

void Entity::destroy(QOpenGLContext *context)
{
    m_dirty ^= Dirty_Destroy;
    for(auto it = m_components.cbegin(); it != m_components.cend(); ++it)
    {
        if((*it)->dirty() & Dirty_Destroy)
            (*it)->destroy(context);
    }

    for(auto childIt = m_children.begin(); childIt != m_children.end(); ++childIt)
    {
        (*childIt)->create(context);
    }
}


} // namespace photon
