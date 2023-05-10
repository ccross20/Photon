#ifndef PHOTON_SCENEOBJECTMODEL_H
#define PHOTON_SCENEOBJECTMODEL_H

#include <QObject>
#include "photon-global.h"

namespace photon {

class Entity;
class ModelModifier;

class SceneObjectModel : public QObject
{
    Q_OBJECT
public:
    explicit SceneObjectModel(SceneObject *sceneObj, QObject *parent = nullptr);
    virtual ~SceneObjectModel();

    Entity *entity() const;
    SceneObject *sceneObject() const;

    virtual void preDraw(double elapsed);
    virtual void updateFromDMX(const DMXMatrix &, const double elapsed);

signals:

protected:
    void setEntity(Entity *);
    virtual void metadataUpdated();

private slots:
    void transformUpdated();
    void metadataChanged();
    void childWasAdded(photon::SceneObject *child);
    void childWasRemoved(photon::SceneObject *child);

private:
    void recursiveAddModifiers(Entity *);
    QVector<ModelModifier*> m_modifiers;
    QVector<SceneObjectModel*> m_children;
    SceneObject *m_sceneObj;
    Entity *m_entity = nullptr;

};

} // namespace photon

#endif // PHOTON_SCENEOBJECTMODEL_H
