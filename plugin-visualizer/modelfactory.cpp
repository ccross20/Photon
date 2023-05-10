#include "modelfactory.h"
#include "fixturemodel.h"
#include "scene/sceneobject.h"
#include "trussmodel.h"

namespace photon {

ModelFactory::ModelFactory()
{

}

SceneObjectModel* ModelFactory::createModel(SceneObject* t_sceneObj)
{
    if(t_sceneObj->typeId() == "fixture")
    {
        return new FixtureModel(t_sceneObj);
    }
    if(t_sceneObj->typeId() == "group")
    {
        return new SceneObjectModel(t_sceneObj);
    }
    if(t_sceneObj->typeId() == "truss")
    {
        return new TrussModel(t_sceneObj);
    }

    return nullptr;
}

} // namespace photon
