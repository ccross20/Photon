
#include <QJsonDocument>
#include <QJsonObject>
#include <QWidget>
#include "sceneobject_p.h"
#include "scenefactory.h"
#include "sceneiterator.h"

namespace photon {

const QByteArray SceneObject::SceneObjectMime = "photon.core.scene-object";

SceneObject::Impl::Impl(SceneObject *t_facade) : facade(t_facade)
{
    uniqueId = QUuid::createUuid().toByteArray();
}

void SceneObject::Impl::rebuildMatrix()
{
    localMatrix = QMatrix4x4{};
    localMatrix.translate(position);
    localMatrix.rotate(QQuaternion::fromEulerAngles(rotation));

    emit facade->matrixChanged();
}

void SceneObject::Impl::addChild(SceneObject *object, int index)
{
    if(object->parent() == facade)
        return;
    if(index < 0)
        index = children.length();
    index = qMin(index, children.length());
    object->m_impl->index = index;
    emit facade->childWillBeAdded(facade, object);

    children.insert(index, object);
    for(int i = index+1; i < children.length(); ++i)
        children[i]->m_impl->index = i;
    object->setParent(facade);


    connect(object, &SceneObject::descendantAdded, facade, &SceneObject::descendantAdded);
    connect(object, &SceneObject::descendantRemoved, facade, &SceneObject::descendantRemoved);
    emit facade->childWasAdded(object);
    emit facade->descendantAdded(object);
}

void SceneObject::Impl::moveChild(SceneObject *object, int index)
{
    if(object->parent() != facade)
        return;
    if(index < 0)
        index = children.length();

    /*
    qDebug() << "Before" << index;
    for(int i = 0; i < children.length(); ++i)
    {
        qDebug() << children[i]->name() << i;
    }
    */

    index = qMin(index, children.length());
    int currentIndex = object->index();
    int targetIndex = index;



    emit facade->childWillBeMoved(object, targetIndex);

    if(currentIndex < targetIndex)
        targetIndex -= 1;

    object->m_impl->index = index;

    children.move(currentIndex,targetIndex);
    for(int i = 0; i < children.length(); ++i)
    {
        children[i]->m_impl->index = i;
    }

    emit facade->childWasMoved(object);
}

void SceneObject::Impl::removeChild(SceneObject *object)
{
    emit facade->childWillBeRemoved(facade, object);
    children.removeOne(object);
    for(int i = object->index(); i >= 0 && i < children.length(); ++i)
        children[i]->m_impl->index = i;

    disconnect(object, &SceneObject::descendantAdded, facade, &SceneObject::descendantAdded);
    disconnect(object, &SceneObject::descendantRemoved, facade, &SceneObject::descendantRemoved);
    emit facade->childWasRemoved(object);
    emit facade->descendantRemoved(object);
}

void SceneObject::Impl::reindexChildren()
{
    for(int i = 0; i < children.length(); ++i)
        children[i]->m_impl->index = i;
}

SceneObject::SceneObject(const QByteArray &t_typeId, SceneObject *t_parent)
    : QObject{t_parent},m_impl(new Impl(this))
{
    m_impl->typeId = t_typeId;
}

SceneObject::~SceneObject()
{
    delete m_impl;
}

QWidget *SceneObject::createEditor()
{
    return new QWidget();
}

SceneObject *SceneObject::clone() const
{
    SceneObject *newObj = SceneFactory::createObject(typeId());

    QJsonObject json;

    if(newObj)
    {
        writeToJson(json);
        newObj->readFromJson(json);

        auto cloned = SceneIterator::ToList(newObj);
        for(auto obj : cloned)
            obj->generateNewUniqueId();
    }

    return newObj;
}

void SceneObject::generateNewUniqueId()
{
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
}

void SceneObject::setParentSceneObject(photon::SceneObject *t_object, int t_index)
{
    if(parentSceneObject())
        parentSceneObject()->m_impl->removeChild(this);

    if(t_object)
        t_object->m_impl->addChild(this, t_index);
}

void SceneObject::setPosition(const QVector3D &t_value)
{
    if(m_impl->position == t_value)
        return;
    m_impl->position = t_value;
    m_impl->rebuildMatrix();
    emit positionChanged();
}
void SceneObject::setRotation(const QVector3D &t_value)
{
    if(m_impl->rotation == t_value)
        return;
    m_impl->rotation = t_value;
    m_impl->rebuildMatrix();
    emit rotationChanged();
}

void SceneObject::setName(const QString &t_value)
{
    if(m_impl->name == t_value)
        return;
    m_impl->name = t_value;
    emit metadataChanged();
}

QVector3D SceneObject::position() const
{
    return m_impl->position;
}

QVector3D SceneObject::rotation() const
{
    return m_impl->rotation;
}

QVector3D SceneObject::globalPosition() const
{
    return globalMatrix().map(QVector3D{});
}

const QMatrix4x4 &SceneObject::localMatrix() const
{
    return m_impl->localMatrix;
}

QMatrix4x4 SceneObject::globalMatrix() const
{
    if(parent())
        return parentSceneObject()->globalMatrix() * m_impl->localMatrix;
    else
        return localMatrix();
}

QString SceneObject::name() const
{
    return m_impl->name;
}

QByteArray SceneObject::uniqueId() const
{
    return m_impl->uniqueId;
}

QByteArray SceneObject::typeId() const
{
    return m_impl->typeId;
}

int SceneObject::index() const
{
    return m_impl->index;
}

int SceneObject::childCount() const
{
    return m_impl->children.length();
}

void SceneObject::moveChildToIndex(SceneObject *child, int index)
{
    m_impl->moveChild(child, index);
}

SceneObject *SceneObject::childAtIndex(int t_index) const
{
    return m_impl->children[t_index];
}

const QVector<SceneObject*> &SceneObject::sceneChildren() const
{
    return m_impl->children;
}

SceneObject *SceneObject::parentSceneObject() const
{
    return static_cast<SceneObject*>(parent());
}

void SceneObject::readFromJson(const QJsonObject &t_json)
{

    m_impl->name = t_json.value("name").toString();
    m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();
    m_impl->typeId = t_json.value("typeId").toString().toLatin1();
    QJsonObject positionObj = t_json.value("position").toObject();
    m_impl->position = QVector3D{static_cast<float>(positionObj.value("x").toDouble()),
                                    static_cast<float>(positionObj.value("y").toDouble()),
                                    static_cast<float>(positionObj.value("z").toDouble())};
    QJsonObject rotationObj = t_json.value("rotation").toObject();
    m_impl->rotation = QVector3D{static_cast<float>(rotationObj.value("x").toDouble()),
                                    static_cast<float>(rotationObj.value("y").toDouble()),
                                    static_cast<float>(rotationObj.value("z").toDouble())};

    if(t_json.contains("children"))
    {
        auto childrenArray = t_json.value("children").toArray();
        for(auto child : childrenArray)
        {
            auto childObj = child.toObject();
            auto sceneChild = SceneFactory::createObject(childObj.value("typeId").toString().toLatin1());
            sceneChild->readFromJson(childObj);
            m_impl->addChild(sceneChild,-1);
        }
    }


}

void SceneObject::writeToJson(QJsonObject &t_json) const
{

    t_json.insert("name", m_impl->name);
    t_json.insert("uniqueId", QString(m_impl->uniqueId));
    t_json.insert("typeId", QString(m_impl->typeId));
    QJsonObject positionObj;
    positionObj.insert("x", m_impl->position.x());
    positionObj.insert("y", m_impl->position.y());
    positionObj.insert("z", m_impl->position.z());
    t_json.insert("position", positionObj);

    QJsonObject rotationObj;
    rotationObj.insert("x", m_impl->rotation.x());
    rotationObj.insert("y", m_impl->rotation.y());
    rotationObj.insert("z", m_impl->rotation.z());
    t_json.insert("rotation", rotationObj);

    if(!m_impl->children.isEmpty())
    {
        QJsonArray childrenArray;
        for(auto child : m_impl->children)
        {
            QJsonObject childObj;
            child->writeToJson(childObj);
            childrenArray.append(childObj);
        }
        t_json.insert("children", childrenArray);
    }

}

} // namespace photon
