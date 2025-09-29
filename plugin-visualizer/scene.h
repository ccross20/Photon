#ifndef PHOTON_SCENE_H
#define PHOTON_SCENE_H

#include <QElapsedTimer>
#include <QObject>
#include "entity.h"
#include "data/dmxmatrix.h"

namespace photon {

class SpotLight;
class InfiniteLight;
class SceneObjectModel;
class Fixture;

class Scene : public QObject
{
    Q_OBJECT
public:
    explicit Scene(QObject *parent = nullptr);
    ~Scene();

    Entity *root() const{return m_rootEntity;}
    Camera *camera() const{return m_camera;}

    void addFixture(Fixture *, Entity * = nullptr);
    void setSceneRoot(SceneObject *);

    virtual void create(QOpenGLContext *context);
    virtual void rebuild(QOpenGLContext *context);
    virtual void draw(DrawContext *context);
    virtual void destroy(QOpenGLContext *context);
    void updateDMX(const DMXMatrix &, double elapsed);

signals:

private slots:
    void componentAdded(photon::AbstractComponent*);
    void componentRemoved(photon::AbstractComponent*);
    void descendantAdded(photon::Entity *);
    void descendantRemoved(photon::Entity *);

private:
    void recursiveAddObject(SceneObject*, Entity*);
    Entity *m_rootEntity = nullptr;
    Camera *m_camera;
    SceneObject *m_sceneRoot;
    SceneObjectModel *m_sceneRootObject;
    QVector<SpotLight*> m_spotLights;
    QVector<InfiniteLight*> m_infiniteLights;
    QVector<SceneObjectModel*> m_models;

    QElapsedTimer m_timer;
    uint m_lightBlock = 0;
    int m_bufferSize = 0;
    std::byte *m_buffer = nullptr;
};

} // namespace photon

#endif // PHOTON_SCENE_H
