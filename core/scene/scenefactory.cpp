#include "scenefactory.h"
#include "fixture/fixture.h"
#include "scenegroup.h"
#include "truss.h"
#include "pixel/pixelstrip.h"
#include "scene/scenearrow.h"

namespace photon {

SceneFactory::SceneFactory()
{

}

SceneObject *SceneFactory::createObject(const QByteArray &id)
{
    qDebug() << "create" << id;
    if(id == "fixture")
        return new Fixture();
    if(id == "group")
        return new SceneGroup();
    if(id == "truss")
        return new Truss();
    if(id == "pixelstrip")
        return new PixelStrip();
    if(id == "arrow")
        return new SceneArrow();

    return nullptr;
}

} // namespace photon
