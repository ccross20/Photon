#ifndef PHOTON_BASE3D_H
#define PHOTON_BASE3D_H


#include "plugin-visualizer-global.h"
#include "component/transformcomponent.h"

namespace photon {



class Entity : public QObject
{
    Q_OBJECT

public:
    Entity(Entity *parent = nullptr);

    Entity *parent() const;
    void setParent(Entity *parent);
    Entity *root() const;

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

    template<class V>
    void gatherComponents(QVector<V> &list) const
    {
        auto comp = findComponent<V>();
        if(comp)
        {
            list.append(comp);
        }

        for(auto it = m_children.cbegin(); it != m_children.cend(); ++it)
        {
            (*it)->gatherComponents(list);
        }
    }

    void setDirty(int dirty);
    int dirty() const;
    QString name() const;
    void setName(const QString &);
    TransformComponent *transform() const{return m_transform;}

    QVector<Entity*> &children(){return m_children;}
    void addChild(Entity *);
    void removeChild(Entity *);
    void gatherFamily(QVector<Entity*> &list) const;
    const QVector<AbstractComponent*> &components() const{return m_components;}

    virtual void create(QOpenGLContext *context);
    virtual void rebuild(QOpenGLContext *context);
    virtual void draw(DrawContext *context);
    virtual void destroy(QOpenGLContext *context);

signals:
    void componentAddedToDescendant(photon::AbstractComponent *);
    void componentRemovedFromDescendant(photon::AbstractComponent *);
    void componentAdded(photon::AbstractComponent *);
    void componentRemoved(photon::AbstractComponent *);
    void descendantAdded(photon::Entity *);
    void descendantRemoved(photon::Entity *);

private:
    QVector<Entity*> m_children;
    QVector<AbstractComponent*> m_components;
    Entity *m_parent = nullptr;
    QString m_name;
    TransformComponent *m_transform = nullptr;
    int m_dirty = Dirty_Init | Dirty_Rebuild | Dirty_Draw;
};

} // namespace photon

#endif // PHOTON_BASE3D_H
