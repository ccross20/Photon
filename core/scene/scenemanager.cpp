#include "scenemanager.h"
#include "sceneobject.h"
#include "sceneiterator.h"
#include "fixture/fixture.h"

namespace photon {

class SceneManager::Impl
{
public:
    SceneObject *root;
    int nextFixtureIndex = 0;
};

SceneManager::SceneManager():m_impl(new Impl)
{
    m_impl->root = new SceneObject("root");

    connect(m_impl->root, &SceneObject::descendantAdded, this, &SceneManager::descendantAdded);
    connect(m_impl->root, &SceneObject::descendantRemoved, this, &SceneManager::descendantRemoved);
}

SceneManager::~SceneManager()
{
    delete m_impl;
}

SceneObject *SceneManager::findObjectById(const QByteArray &t_id)
{
    return SceneIterator::FindOne(m_impl->root,[t_id](SceneObject *t_object){return t_object->uniqueId() == t_id;});
}

SceneObject *SceneManager::rootObject() const
{
    return m_impl->root;
}

void SceneManager::descendantAdded(photon::SceneObject *t_object)
{
    auto fixture = dynamic_cast<Fixture*>(t_object);
    if(fixture)
    {
        fixture->setUniqueIndex(m_impl->nextFixtureIndex++);
        qDebug() << "Fixture added";
    }
}

void SceneManager::descendantRemoved(photon::SceneObject *t_object)
{

}

void SceneManager::readFromJson(const QJsonObject &json, const LoadContext &t_context)
{
    if(json.contains("root"))
    {
        m_impl->root->readFromJson(json.value("root").toObject(), t_context);
    }

    m_impl->nextFixtureIndex = json.value("nextFixtureIndex").toInt();

}

void SceneManager::writeToJson(QJsonObject &json) const
{


    QJsonObject rootObj;
    m_impl->root->writeToJson(rootObj);
    json.insert("root", rootObj);
    json.insert("nextFixtureIndex", m_impl->nextFixtureIndex);
}

} // namespace photon
