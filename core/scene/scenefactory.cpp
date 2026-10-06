#include "scenefactory.h"
#include "fixture/fixture.h"
#include "scenegroup.h"
#include "truss.h"
#include "scenesurface.h"
#include "scenezone.h"
#include "pixel/pixelstrip.h"
#include "scene/scenearrow.h"
#include "scene/scenedirection.h"
#include "scene/sceneaxis.h"
#include "scene/sceneboundaryrectangle.h"
#include "scene/sceneboundaryoval.h"
#include "scene/scenepointmarker.h"
#include "scene/scenefalloff.h"
#include "scene/scenebox.h"
#include "scene/sceneambientlight.h"
#include "scene/scenedirectionallight.h"

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
    if(id == "surface")
        return new SceneSurface();
    if(id == "directionallight")
        return new SceneDirectionalLight();
    if(id == "ambientlight")
        return new SceneAmbientLight();
    if(id == "box")
        return new SceneBox();
    if(id == "zone")
        return new SceneZone();
    if(id == "pixelstrip")
        return new PixelStrip();
    if(id == "arrow")
        return new SceneArrow();
    if(id == "direction")
        return new SceneDirection();
    if(id == "axis")
        return new SceneAxis();
    if(id == "boundaryrectangle")
        return new SceneBoundaryRectangle();
    if(id == "boundaryoval")
        return new SceneBoundaryOval();
    if(id == "pointmarker")
        return new ScenePointMarker();
    // "linearfalloff" is the Linear Falloff that Falloff replaced.
    if(id == "falloff" || id == "linearfalloff")
        return new SceneFalloff();

    return nullptr;
}

} // namespace photon
