#ifndef PHOTON_SCENE_H
#define PHOTON_SCENE_H

#include <QObject>
#include "entity.h"

namespace photon {

class SpotLight;
class InfiniteLight;

class Scene : public QObject
{
    Q_OBJECT
public:
    explicit Scene(QObject *parent = nullptr);
    ~Scene();

    Entity *root() const{return m_rootEntity;}
    Camera *camera() const{return m_camera;}

    virtual void create(QOpenGLContext *context);
    virtual void rebuild(QOpenGLContext *context);
    virtual void draw(DrawContext *context);
    virtual void destroy(QOpenGLContext *context);

signals:

private slots:
    void componentAdded(photon::AbstractComponent*);
    void componentRemoved(photon::AbstractComponent*);
    void descendantAdded(photon::Entity *);
    void descendantRemoved(photon::Entity *);

private:
    Entity *m_rootEntity = nullptr;
    Camera *m_camera;
    QVector<SpotLight*> m_spotLights;
    QVector<InfiniteLight*> m_infiniteLights;
    uint m_lightBlock = 0;
    byte *m_buffer = nullptr;
};

} // namespace photon

#endif // PHOTON_SCENE_H
