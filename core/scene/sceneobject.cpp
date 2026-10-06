
#include <QJsonDocument>
#include <QJsonObject>
#include <QWidget>
#include <QQuaternion>
#include <QRegularExpression>
#include <QSet>
#include "sceneobject_p.h"
#include "scenefactory.h"
#include "sceneiterator.h"
#include "photoncore.h"
#include "util/utils.h"

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
    connect(object, &SceneObject::descendantModified, facade, &SceneObject::descendantModified);
    connect(object, &SceneObject::metadataChanged, facade, &SceneObject::descendantModified);
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
    disconnect(object, &SceneObject::descendantModified, facade, &SceneObject::descendantModified);
    disconnect(object, &SceneObject::metadataChanged, facade, &SceneObject::descendantModified);
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

    // metadataChanged is the scene's long-standing "something displayable
    // changed" signal; republish it to the resource notifier so the project
    // panel sees renames, tag edits and visibility toggles alike. Calls the
    // base implementation explicitly - the override redirects back into
    // metadataChanged, which would otherwise recurse.
    connect(this, &SceneObject::metadataChanged, this, [this](){
        ProjectResource::notifyResourceChanged();
    });
}

SceneObject::~SceneObject()
{
    delete m_impl;
}

QWidget *SceneObject::createEditor()
{
    return new QWidget();
}

bool SceneObject::isVisible() const
{
    return m_impl->visible;
}

void SceneObject::setVisible(bool t_value)
{
    if(m_impl->visible == t_value)
        return;
    m_impl->visible = t_value;
    emit metadataChanged(this);
}


SceneObject *SceneObject::clone() const
{
    SceneObject *newObj = SceneFactory::createObject(typeId());

    LoadContext c;
    c.project = photonApp->project();
    QJsonObject json;

    if(newObj)
    {
        writeToJson(json);
        newObj->readFromJson(json, c);

        auto cloned = SceneIterator::ToList(newObj);
        for(auto obj : cloned)
            obj->generateNewUniqueId();
    }

    return newObj;
}

QString SceneObject::nextAvailableName(const QString &t_name, const QSet<QString> &t_taken)
{
    static const QRegularExpression numbered(QStringLiteral("^(.*?)(\\s*)(\\d+)$"));

    const QString name = t_name.trimmed();
    QString stem = name;
    QString separator = QStringLiteral(" ");
    int width = 0;

    const QRegularExpressionMatch match = numbered.match(name);
    if(match.hasMatch())
    {
        stem = match.captured(1);
        separator = match.captured(2);
        const QString digits = match.captured(3);
        if(digits.startsWith('0'))
            width = digits.size();
    }

    // The bare stem counts as number 1, so copying "Spot" gives "Spot 2".
    const QRegularExpression sibling("^" + QRegularExpression::escape(stem) + "\\s*(\\d+)$");
    qint64 highest = 0;
    for(const QString &taken : t_taken)
    {
        if(!stem.isEmpty() && taken == stem)
            highest = std::max<qint64>(highest, 1);
        else if(const auto m = sibling.match(taken); m.hasMatch())
            highest = std::max(highest, m.captured(1).toLongLong());
    }

    qint64 number = std::max<qint64>(highest + 1, 2);
    QString candidate;
    do
        candidate = stem + separator + QString::number(number++).rightJustified(width, '0');
    while(t_taken.contains(candidate));
    return candidate;
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
    emit metadataChanged(this);
}

void SceneObject::triggerUpdate()
{
    emit metadataChanged(this);
}

QVector3D SceneObject::position() const
{
    return m_impl->position;
}

QVector3D SceneObject::rotation() const
{
    return m_impl->rotation;
}

QVector3D SceneObject::globalRotation() const
{
    // Decompose the rotation out of the global matrix (its basis columns, normalised
    // to strip any scale) as Euler angles. NOTE: the old implementation mapped the
    // local Euler vector through the full matrix as if it were a point, which folded
    // in translation and produced garbage (e.g. it returned the position when the
    // local rotation was zero).
    const QMatrix4x4 g = globalMatrix();
    QVector3D c0(g(0, 0), g(1, 0), g(2, 0));
    QVector3D c1(g(0, 1), g(1, 1), g(2, 1));
    QVector3D c2(g(0, 2), g(1, 2), g(2, 2));
    c0.normalize(); c1.normalize(); c2.normalize();
    const float v[9] = {
        c0.x(), c1.x(), c2.x(),
        c0.y(), c1.y(), c2.y(),
        c0.z(), c1.z(), c2.z()
    };
    return QQuaternion::fromRotationMatrix(QMatrix3x3(v)).toEulerAngles();
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

void SceneObject::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
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

    readResourceJson(t_json);
    m_impl->visible = t_json.contains("visible") ? t_json.value("visible").toBool() : true;

    m_impl->rebuildMatrix();
    if(t_json.contains("children"))
    {
        auto childrenArray = t_json.value("children").toArray();
        for(auto child : childrenArray)
        {
            auto childObj = child.toObject();
            auto sceneChild = SceneFactory::createObject(childObj.value("typeId").toString().toLatin1());
            sceneChild->readFromJson(childObj, t_context);
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

    writeResourceJson(t_json);
    t_json.insert("visible", m_impl->visible);

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
