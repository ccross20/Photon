#ifndef SCENEOBJECT_P_H
#define SCENEOBJECT_P_H

#include <QVector3D>
#include <QMatrix4x4>
#include "sceneobject.h"

namespace photon {

class SceneObject::Impl
{
public:
    Impl(SceneObject *);
    void rebuildMatrix();
    void addChild(SceneObject *object, int index);
    void moveChild(SceneObject *object, int index);
    void removeChild(SceneObject *object);
    void reindexChildren();

    QVector<SceneObject*> children;
    QString name;
    QByteArray typeId;
    QByteArray uniqueId;
    QVector3D position;
    QVector3D rotation;
    QMatrix4x4 localMatrix;
    QStringList tags;
    SceneObject *facade;
    int index = -1;
    int targetIndex;
};

}

#endif // SCENEOBJECT_P_H
