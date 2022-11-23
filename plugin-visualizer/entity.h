#ifndef PHOTON_BASE3D_H
#define PHOTON_BASE3D_H


#include "plugin-visualizer-global.h"

namespace photon {



class Entity
{
public:
    Entity(Entity *parent = nullptr);

    Entity *parent() const;
    void setParent(Entity *parent);

    void addComponent(AbstractComponent *);
    void removeComponent(AbstractComponent *);

    template<class V>
    V findComponent() const
    {
        for(auto it = m_components.cbegin(); it != m_components.cend(); ++it)
        {
            auto comp = dynamic_cast<V>(*it);
            if(comp)
                return comp;
        }
        return nullptr;
    }

    void setDirty(int dirty);
    int dirty() const;
    QString name() const;
    void setName(const QString &);

    QVector<Entity*> &children(){return m_children;}

    virtual void create(QOpenGLContext *context);
    virtual void rebuild(QOpenGLContext *context);
    virtual void draw(DrawContext *context);
    virtual void destroy(QOpenGLContext *context);

private:
    QVector<Entity*> m_children;
    QVector<AbstractComponent*> m_components;
    Entity *m_parent = nullptr;
    QString m_name;
    int m_dirty = Dirty_Init | Dirty_Rebuild | Dirty_Draw;
};

} // namespace photon

#endif // PHOTON_BASE3D_H
