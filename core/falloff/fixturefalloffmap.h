#ifndef PHOTON_FIXTUREFALLOFFMAP_H
#define PHOTON_FIXTUREFALLOFFMAP_H

#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT SceneFalloffMap : public QObject
{
    Q_OBJECT
public:
    explicit SceneFalloffMap(QObject *parent = nullptr);
    virtual ~SceneFalloffMap();

    void addSceneObject(SceneObject *sceneObject, double delay);
    virtual double falloff(SceneObject *) const;

    virtual void readFromJson(const QJsonObject &);
    virtual void writeToJson(QJsonObject &) const;

signals:

    void Updated();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTUREFALLOFFMAP_H
