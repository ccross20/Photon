#ifndef PHOTON_MODELLOADER_H
#define PHOTON_MODELLOADER_H
#include <QString>

namespace photon {

class Entity;

class ModelLoader
{
public:
    ModelLoader();

    Entity *loadResource(QString);
};

} // namespace photon

#endif // PHOTON_MODELLOADER_H
