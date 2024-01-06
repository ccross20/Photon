#include "arrowmodel.h"
#include "modelloader.h"
#include "scene/sceneobject.h"
#include "entity.h"
#include "scene/scenearrow.h"

namespace photon {

ArrowModel::ArrowModel(SceneObject *t_sceneObj, QObject *t_parent) : SceneObjectModel(t_sceneObj, t_parent)
{
    ModelLoader loader;
    auto entity = loader.loadResource("C:\\Projects\\photon\\src\\plugin-visualizer\\resources\\model\\arrow.fbx");


    TransformComponent *xform = entity->findComponent<TransformComponent*>();

    float scale = static_cast<SceneArrow*>(t_sceneObj)->size();
    xform->setScale(QVector3D{.01f * scale,.01f * scale,.01f * scale});

    setEntity(entity);

}

void ArrowModel::metadataUpdated()
{

    TransformComponent *xform = entity()->findComponent<TransformComponent*>();

    float scale = static_cast<SceneArrow*>(sceneObject())->size();
    xform->setScale(QVector3D{.01f * scale,.01f * scale,.01f * scale});
}

} // namespace photon
