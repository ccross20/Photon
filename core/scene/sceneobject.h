#ifndef PHOTON_SCENEOBJECT_H
#define PHOTON_SCENEOBJECT_H

#include <QObject>
#include "photon-global.h"
#include "project/projectresource.h"

namespace photon {


class PHOTONCORE_EXPORT SceneObject : public QObject, public ProjectResource
{
    Q_OBJECT
public:
    const static QByteArray SceneObjectMime;

    explicit SceneObject(const QByteArray &typeId, SceneObject *parent = nullptr);
    virtual ~SceneObject();

    SceneObject *clone() const;
    QVector3D position() const;
    QVector3D rotation() const;
    QVector3D globalPosition() const;
    QVector3D globalRotation() const;
    const QMatrix4x4 &localMatrix() const;
    QMatrix4x4 globalMatrix() const;

    QString name() const;
    QByteArray uniqueId() const;
    QByteArray typeId() const;

    // Tags live on ProjectResource so every resource type has them; these are
    // the long-standing scene-object spellings, kept as aliases.
    const QStringList &tags() const{return resourceTags();}
    void setTags(const QStringList &t_tags){setResourceTags(t_tags);}
    void addTag(const QString &t_tag){addResourceTag(t_tag);}
    void removeTag(const QString &t_tag){removeResourceTag(t_tag);}

    // ProjectResource
    QByteArray resourceId() const override{return uniqueId();}
    QByteArray resourceTypeId() const override{return typeId();}
    QString resourceName() const override{return name();}
    void setResourceName(const QString &t_name) override{setName(t_name);}
    QObject *resourceObject() override{return this;}
    QWidget *createResourceEditor() override{return createEditor();}
    // Route shared-metadata changes through the scene's own signal; the
    // constructor bridges that back out to the resource notifier.
    void notifyResourceChanged() override{emit metadataChanged(this);}

    // Whether this object (and, by extension, its subtree) is drawn in the
    // visualizer. Purely a display toggle — does not affect DMX/evaluation.
    bool isVisible() const;

    SceneObject *parentSceneObject() const;

    void generateNewUniqueId();
    void moveChildToIndex(SceneObject *child, int index);
    int childCount() const;
    SceneObject *childAtIndex(int index) const;
    int index() const;
    const QVector<SceneObject*> &sceneChildren() const;

    virtual QWidget *createEditor();

    virtual void readFromJson(const QJsonObject &, const LoadContext &);
    virtual void writeToJson(QJsonObject &) const;

    void triggerUpdate();

public slots:
    void setParentSceneObject(photon::SceneObject *, int index = -1);

    void setPosition(const QVector3D &);
    void setRotation(const QVector3D &);
    void setName(const QString &);
    void setVisible(bool);

signals:
    void positionChanged();
    void rotationChanged();
    void matrixChanged();
    void metadataChanged(photon::SceneObject *);

    void childWillBeAdded(photon::SceneObject *parent, photon::SceneObject *child);
    void childWasAdded(photon::SceneObject *child);
    void childWillBeRemoved(photon::SceneObject *parent, photon::SceneObject *child);
    void childWasRemoved(photon::SceneObject *child);
    void childWillBeMoved(photon::SceneObject *child, int newIndex);
    void childWasMoved(photon::SceneObject *child);
    void descendantAdded(photon::SceneObject *);
    void descendantRemoved(photon::SceneObject *);
    void descendantModified(photon::SceneObject *);


private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_SCENEOBJECT_H
