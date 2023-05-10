#ifndef PHOTON_MODELFACTORY_H
#define PHOTON_MODELFACTORY_H


namespace photon {

class SceneObjectModel;
class SceneObject;

class ModelFactory
{
public:
    ModelFactory();

    static SceneObjectModel* createModel(SceneObject*);
};

} // namespace photon

#endif // PHOTON_MODELFACTORY_H
