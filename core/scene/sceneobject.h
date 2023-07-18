#ifndef PHOTON_SCENEOBJECT_H
#define PHOTON_SCENEOBJECT_H

#include <QObject>
#include "photon-global.h"

namespace photon {


class PHOTONCORE_EXPORT SceneObject : public QObject
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

    SceneObject *parentSceneObject() const;

    void generateNewUniqueId();
    void moveChildToIndex(SceneObject *child, int index);
    int childCount() const;
    SceneObject *childAtIndex(int index) const;
    int index() const;
    const QVector<SceneObject*> &sceneChildren() const;

    virtual QWidget *createEditor();

    virtual void readFromJson(const QJsonObject &);
    virtual void writeToJson(QJsonObject &) const;

    void triggerUpdate();

public slots:
    void setParentSceneObject(photon::SceneObject *, int index = -1);

    void setPosition(const QVector3D &);
    void setRotation(const QVector3D &);
    void setName(const QString &);

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
