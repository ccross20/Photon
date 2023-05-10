#include "fixturemodel.h"
#include "entity.h"
#include "modifier/panmodelmodifier.h"
#include "modifier/tiltmodelmodifier.h"
#include "modifier/lampmodifier.h"
#include "component/spotlight.h"
#include "modelloader.h"
#include "scene/sceneobject.h"

namespace photon {

FixtureModel::FixtureModel(SceneObject *t_sceneObj, QObject *t_parent) : SceneObjectModel(t_sceneObj, t_parent)
{
    ModelLoader loader;
    auto entity = loader.loadResource("C:\\Projects\\photon\\src\\plugin-visualizer\\resources\\model\\moving-head.fbx");


    TransformComponent *xform = entity->findComponent<TransformComponent*>();

    xform->setScale(QVector3D{.01,.01,.01});

    setEntity(entity);

    recursiveAddModifiers(entity);
}

void FixtureModel::recursiveAddModifiers(Entity *t_entity)
{
    if(t_entity->name().startsWith("pan_"))
    {
        m_modifiers.append(new PanModelModifier(this, t_entity));
    }

    if(t_entity->name().startsWith("tilt_"))
    {
        m_modifiers.append(new TiltModelModifier(this, t_entity));
    }

    if(t_entity->name() == "lamp")
    {
        SpotLight *light = new SpotLight;
        light->setColor(QColor::fromRgbF(.6,.2,.6));
        light->setAngle(25.0);
        light->setHardness(.8);
        t_entity->addComponent(light);
        m_modifiers.append(new LampModifier(this, t_entity));
    }

    for(auto child : t_entity->children())
    {
        recursiveAddModifiers(child);
    }
}

void FixtureModel::preDraw(double t_elapsed)
{
    SceneObjectModel::preDraw(t_elapsed);
    for(auto modifier : m_modifiers)
    {
        modifier->preDraw(t_elapsed);
    }
}

void FixtureModel::updateFromDMX(const DMXMatrix &t_matrix, const double t_elapsed)
{
    SceneObjectModel::updateFromDMX(t_matrix, t_elapsed);

    //qDebug() << sceneObject()->name();

    for(auto modifier : m_modifiers)
    {
        modifier->updateModel(t_matrix, t_elapsed);
    }
}

} // namespace photon
