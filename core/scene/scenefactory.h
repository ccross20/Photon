#ifndef PHOTON_SCENEFACTORY_H
#define PHOTON_SCENEFACTORY_H


#include "photon-global.h"
namespace photon {

class SceneFactory
{
public:
    SceneFactory();

    static SceneObject *createObject(const QByteArray &id);
};

} // namespace photon

#endif // PHOTON_SCENEFACTORY_H
