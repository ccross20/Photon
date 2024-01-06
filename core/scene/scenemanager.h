#ifndef PHOTON_SCENEMANAGER_H
#define PHOTON_SCENEMANAGER_H

#include "photon-global.h"

namespace photon {

class SceneManager : public QObject
{
public:
    SceneManager();
    ~SceneManager();

    SceneObject *findObjectById(const QByteArray&);
    SceneObject *rootObject() const;

    void readFromJson(const QJsonObject &json, const LoadContext &);
    void writeToJson(QJsonObject &json) const;

private slots:
    void descendantAdded(photon::SceneObject *);
    void descendantRemoved(photon::SceneObject *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SCENEMANAGER_H
