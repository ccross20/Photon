#include "scenemanager.h"
#include "sceneobject.h"
#include "sceneiterator.h"

namespace photon {

class SceneManager::Impl
{
public:
    SceneObject *root;
};

SceneManager::SceneManager():m_impl(new Impl)
{
    m_impl->root = new SceneObject("root");
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


void SceneManager::readFromJson(const QJsonObject &json)
{
    if(json.contains("root"))
    {
        m_impl->root->readFromJson(json.value("root").toObject());
    }

}

void SceneManager::writeToJson(QJsonObject &json) const
{


    QJsonObject rootObj;
    m_impl->root->writeToJson(rootObj);
    json.insert("root", rootObj);
}

} // namespace photon
