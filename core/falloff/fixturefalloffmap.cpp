#include "fixturefalloffmap.h"

namespace photon {

class SceneFalloffMap::Impl
{
public:
    QHash<SceneObject*,double> hash;
};

SceneFalloffMap::SceneFalloffMap(QObject *parent)
    : QObject{parent},m_impl(new Impl)
{

}

SceneFalloffMap::~SceneFalloffMap()
{
    delete m_impl;
}

void SceneFalloffMap::addSceneObject(SceneObject *t_sceneObject, double delay)
{
    m_impl->hash.insert(t_sceneObject, delay);

    emit Updated();
}

double SceneFalloffMap::falloff(SceneObject *t_sceneObject) const
{
    return m_impl->hash.value(t_sceneObject, 0.0);
}

void SceneFalloffMap::readFromJson(const QJsonObject &)
{

}

void SceneFalloffMap::writeToJson(QJsonObject &) const
{

}

} // namespace photon
