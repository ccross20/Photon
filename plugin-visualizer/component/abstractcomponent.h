#ifndef PHOTON_ABSTRACTCOMPONENT_H
#define PHOTON_ABSTRACTCOMPONENT_H

#include "plugin-visualizer-global.h"

namespace photon {

class AbstractComponent
{
public:
    AbstractComponent();
    virtual ~AbstractComponent();

    void addEntity(Entity *);
    void removeEntity(Entity *);
    const QVector<Entity*> &entities() const{return m_entities;}

    virtual void create(QOpenGLContext *context);
    virtual void rebuild(QOpenGLContext *context);
    virtual void draw(DrawContext *context);
    virtual void destroy(QOpenGLContext *context);

    void setDirty(int dirty);
    int dirty() const;


private:
    QVector<Entity*> m_entities;
    int m_dirty = Dirty_Init | Dirty_Rebuild;
};

} // namespace photon

#endif // PHOTON_ABSTRACTCOMPONENT_H
