#include "abstractcomponent.h"

namespace photon {

AbstractComponent::AbstractComponent()
{

}

AbstractComponent::~AbstractComponent()
{

}

void AbstractComponent::addEntity(Entity *t_entity)
{
    m_entities.append(t_entity);
}

void AbstractComponent::removeEntity(Entity *t_entity)
{
    m_entities.removeOne(t_entity);
}

void AbstractComponent::setDirty(int t_dirty)
{
    m_dirty |= t_dirty;
}

int AbstractComponent::dirty() const
{
    return m_dirty;
}

void AbstractComponent::create(QOpenGLContext *context)
{
    m_dirty ^= Dirty_Init;
}

void AbstractComponent::rebuild(QOpenGLContext *context)
{
    m_dirty ^= Dirty_Rebuild;
}

void AbstractComponent::draw(DrawContext *context)
{
    m_dirty ^= Dirty_Draw;
}

void AbstractComponent::destroy(QOpenGLContext *context)
{
    m_dirty ^= Dirty_Destroy;
}


} // namespace photon
