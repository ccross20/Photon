#include "entity.h"
#include "component/abstractcomponent.h"

namespace photon {

Entity::Entity(Entity *t_parent):m_parent(t_parent)
{
    if(m_parent)
        m_parent->m_children.append(this);
}

Entity *Entity::parent() const
{
    return m_parent;
}

void Entity::setParent(Entity *t_parent)
{
    if(m_parent)
        m_parent->m_children.removeOne(this);
    m_parent = t_parent;
    if(m_parent)
        m_parent->m_children.append(this);
}

void Entity::addComponent(AbstractComponent *t_component)
{
    m_components.append(t_component);
    t_component->addEntity(this);
}

void Entity::removeComponent(AbstractComponent *t_component)
{
    m_components.removeOne(t_component);
    t_component->removeEntity(this);
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
