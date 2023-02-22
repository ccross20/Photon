
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QImage>

#include "visualizerpanel.h"
#include "photoncore.h"

#include "project/project.h"
#include "fixture/fixturecollection.h"
#include "entity.h"
#include "fixturemodel.h"
#include "graph/bus/busevaluator.h"
#include "data/dmxmatrix.h"
#include "component/planemesh.h"
#include "component/materialcomponent.h"
#include "component/transformcomponent.h"
#include "component/spotlight.h"
#include "component/infinitelight.h"
#include "scene.h"

namespace photon {

VisualizerPanel::VisualizerPanel(): Panel("Visualizer")
{
    m_viewport = new OpenGLViewport;

    setPanelWidget(m_viewport);

    Q_INIT_RESOURCE(resources);


    Scene *scene = new Scene;

    if(photonApp->project())
    {
        for(auto fixture : photonApp->project()->fixtures()->fixtures())
        {
            FixtureModel *model = new FixtureModel(fixture);

            if(model)
                model->entity()->setParent(scene->root());

            m_models.append(model);
        }
    }


    connect(photonApp->busEvaluator(), &BusEvaluator::evaluationCompleted, this, &VisualizerPanel::dmxUpdated);


/*
    Entity *lightEntity = new Entity(scene->root());
    TransformComponent *lightTransform = new TransformComponent;
    lightTransform->setPosition(QVector3D(0,0,0));
    lightTransform->setRotationEuler(QVector3D(0.0,-90,0));
    lightEntity->addComponent(lightTransform);

    SpotLight *light = new SpotLight;
    light->setColor(QColor::fromRgbF(.6,.2,.6));
    light->setAngle(25.0);
    light->setHardness(.8);
    lightEntity->addComponent(light);

    m_transform = lightTransform;
    */






    Entity *infiniteEntity = new Entity(scene->root());

    TransformComponent *infiniteTransform = new TransformComponent;
    infiniteTransform->setPosition(QVector3D(0,0,0));
    infiniteTransform->setRotationEuler(QVector3D(-45,45,0));
    infiniteEntity->addComponent(infiniteTransform);

    InfiniteLight *infinite = new InfiniteLight;
    infinite->setColor(QColor::fromRgbF(.5,.5,.5));
    infiniteEntity->addComponent(infinite);





    Entity *zPlane = new Entity(scene->root());

    TransformComponent *zTransform = new TransformComponent;
    zTransform->setPosition(QVector3D(0,0,0));
    zTransform->setRotationEuler(QVector3D(0,0,0));
    zPlane->addComponent(zTransform);

    PlaneMesh *zMesh = new PlaneMesh;
    zMesh->setXSize(5);
    zMesh->setYSize(5);
    zPlane->addComponent(zMesh);


    MaterialComponent *material = new MaterialComponent;
    material->setDiffuseColor(QColor(255,255,255));
    material->setFragmentSource(":/resources/shader/lighting.frag");
    material->setVertexSource(":/resources/shader/basic_lighting.vert");
    zPlane->addComponent(material);
    m_material = material;


    Entity *xPlane = new Entity(scene->root());

    TransformComponent *xTransform = new TransformComponent;
    xTransform->setPosition(QVector3D(-5,1.5,0));
    xTransform->setRotationEuler(QVector3D(0,0,-90));
    xPlane->addComponent(xTransform);

    PlaneMesh *xMesh = new PlaneMesh;
    xMesh->setXSize(1.5);
    xMesh->setYSize(5);
    xPlane->addComponent(xMesh);
    xPlane->addComponent(material);

    m_viewport->setScene(scene);



}

VisualizerPanel::~VisualizerPanel()
{

}

void VisualizerPanel::mousePressEvent(QMouseEvent *event)
{
    Panel::mousePressEvent(event);

    /*
    m_clickCounter++;


    int rot = m_clickCounter % 4;

    if(rot > 1)
        m_transform->setRotationEuler(QVector3D(0.0,-90,0));
    else
        m_transform->setRotationEuler(QVector3D(0.0,-0,0));
        */
}

void VisualizerPanel::dmxUpdated()
{
    auto dmx = photonApp->busEvaluator()->dmxMatrix();
    for(auto model : m_models)
    {
        model->updateFromDMX(dmx);
    }
}

} // namespace photon
