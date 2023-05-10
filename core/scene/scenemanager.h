#ifndef PHOTON_SCENEMANAGER_H
#define PHOTON_SCENEMANAGER_H

#include "photon-global.h"

namespace photon {

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    SceneObject *findObjectById(const QByteArray&);
    SceneObject *rootObject() const;

    void readFromJson(const QJsonObject &json);
    void writeToJson(QJsonObject &json) const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SCENEMANAGER_H
