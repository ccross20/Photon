#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include "scene.h"
#include "entity.h"
#include "component/spotlight.h"
#include "component/infinitelight.h"
#include "freecamera.h"

namespace photon {

Scene::Scene(QObject *parent)
    : QObject{parent}
{
    m_rootEntity = new Entity;
    m_camera = new FreeCamera;

    connect(m_rootEntity, &Entity::componentAddedToDescendant, this, &Scene::componentAdded);
    connect(m_rootEntity, &Entity::componentRemovedFromDescendant, this, &Scene::componentRemoved);
    connect(m_rootEntity, &Entity::componentAdded, this, &Scene::componentAdded);
    connect(m_rootEntity, &Entity::componentRemoved, this, &Scene::componentRemoved);
    connect(m_rootEntity, &Entity::descendantAdded, this, &Scene::descendantAdded);
    connect(m_rootEntity, &Entity::descendantRemoved, this, &Scene::descendantRemoved);
}

Scene::~Scene()
{
    if(m_buffer)
        free(m_buffer);
}

void Scene::componentAdded(AbstractComponent *t_component)
{
    SpotLight *spot = dynamic_cast<SpotLight*>(t_component);
    if(spot)
        m_spotLights.append(spot);

    InfiniteLight *infinite = dynamic_cast<InfiniteLight*>(t_component);
    if(infinite)
        m_infiniteLights.append(infinite);
}

void Scene::componentRemoved(AbstractComponent *t_component)
{
    SpotLight *spot = dynamic_cast<SpotLight*>(t_component);
    if(spot)
        m_spotLights.removeOne(spot);

    InfiniteLight *infinite = dynamic_cast<InfiniteLight*>(t_component);
    if(infinite)
        m_infiniteLights.removeOne(infinite);
}

void Scene::descendantAdded(photon::Entity *t_entity)
{
    QVector<Entity*> family;
    t_entity->gatherFamily(family);

    for(auto ent : family)
    {
        for(auto comp : ent->components())
        {
            componentAdded(comp);
        }
    }
}

void Scene::descendantRemoved(photon::Entity *t_entity)
{
    QVector<Entity*> family;
    t_entity->gatherFamily(family);

    for(auto ent : family)
    {
        for(auto comp : ent->components())
        {
            componentRemoved(comp);
        }
    }
}

void Scene::create(QOpenGLContext *t_context)
{
    if(!t_context)
        return;

    if(m_lightBlock == 0)
    {
        t_context->functions()->glGenBuffers(1, &m_lightBlock);
        t_context->functions()->glBindBuffer(GL_UNIFORM_BUFFER, m_lightBlock);
        t_context->functions()->glBufferData(GL_UNIFORM_BUFFER, 1416, NULL, GL_STATIC_DRAW);
        t_context->functions()->glBindBuffer(GL_UNIFORM_BUFFER, 0);
        t_context->extraFunctions()->glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_lightBlock, 0, 1416);
    }

    m_rootEntity->create(t_context);
}

void Scene::rebuild(QOpenGLContext *t_context)
{
    if(!t_context)
        return;

    if(!m_buffer)
        m_buffer = static_cast<byte*>(malloc(1416 * sizeof(byte)));

    auto bufferStart = m_buffer;
    uint chunkSize = 4;

    for(int i = 0; i < 10; i++)
    {
        if(i < m_spotLights.length())
        {
            const auto spot = m_spotLights[i];
            QVector3D pos = spot->globalPosition();
            std::memcpy(m_buffer, &pos, sizeof(GLfloat) * 3);
            m_buffer += 4 * chunkSize;

            QVector3D direction = spot->globalDirection();
            std::memcpy(m_buffer, &direction, sizeof(GLfloat) * 3);
            m_buffer += 3 * chunkSize;

            GLfloat cutoff = std::cos(qDegreesToRadians((spot->hardness() * spot->angle())));
            std::memcpy(m_buffer, &cutoff, sizeof(GLfloat));
            m_buffer += chunkSize;
            GLfloat outerCutoff = std::cos(qDegreesToRadians(spot->angle()));
            std::memcpy(m_buffer, &outerCutoff, sizeof(GLfloat));
            m_buffer += chunkSize;

            GLfloat constant = spot->constant();
            GLfloat linear = spot->linear();
            GLfloat quadratic = spot->quadratic();
            std::memcpy(m_buffer, &constant, sizeof(GLfloat));
            m_buffer += chunkSize;
            std::memcpy(m_buffer, &linear, sizeof(GLfloat));
            m_buffer += chunkSize;
            std::memcpy(m_buffer, &quadratic, sizeof(GLfloat));
            m_buffer += chunkSize;

            QVector4D color(spot->color().redF(), spot->color().greenF(), spot->color().blueF(), 1.0);
            std::memcpy(m_buffer, &color, sizeof(float) * 3);
            m_buffer += chunkSize * 4;

            QMatrix4x4 mat = spot->matrix();
            std::memcpy(m_buffer, &mat, sizeof(float) * 16);
            m_buffer += chunkSize * 16;
        }
        else
        {
            m_buffer += 128;
        }
    }

    for(int i = 0; i < 2; i++)
    {
        if(i < m_infiniteLights.length())
        {
            auto infLight = m_infiniteLights[i];
            QVector3D pos = infLight->globalDirection();
            std::memcpy(m_buffer, &pos, sizeof(float) * 3);
            m_buffer += chunkSize * 4;

            QVector3D ambient;
            QVector3D diffuse(infLight->color().redF(), infLight->color().greenF(), infLight->color().blueF());
            std::memcpy(m_buffer, &ambient, sizeof(float) * 3);
            m_buffer += chunkSize * 4;
            std::memcpy(m_buffer, &diffuse, sizeof(float) * 3);
            m_buffer += chunkSize * 4;
            std::memcpy(m_buffer, &diffuse, sizeof(float) * 3);
            m_buffer += chunkSize * 4;
        }
        else
        {
            m_buffer += 64;
        }
    }

    int spotCount = m_spotLights.length();
    std::memcpy(m_buffer, &spotCount, sizeof(int));
    m_buffer += chunkSize;

    int infiniteCount = m_infiniteLights.length();
    std::memcpy(m_buffer, &infiniteCount, sizeof(int));
    m_buffer += chunkSize;

    m_buffer = bufferStart;

    t_context->functions()->glBindBuffer(GL_UNIFORM_BUFFER, m_lightBlock);
    t_context->functions()->glBufferSubData(GL_UNIFORM_BUFFER,0, 1416, m_buffer);
    //t_context->functions()->glBufferData(GL_UNIFORM_BUFFER, 16, m_buffer, GL_STATIC_DRAW);
    t_context->functions()->glBindBuffer(GL_UNIFORM_BUFFER, 0);

    m_rootEntity->rebuild(t_context);
}

void Scene::draw(DrawContext *t_context)
{
    m_rootEntity->draw(t_context);
}

void Scene::destroy(QOpenGLContext *t_context)
{
    m_rootEntity->destroy(t_context);
}


} // namespace photon