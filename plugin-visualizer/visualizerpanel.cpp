
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

namespace photon {

VisualizerPanel::VisualizerPanel(): Panel("Visualizer")
{
    m_viewport = new OpenGLViewport;

    setPanelWidget(m_viewport);


    Entity *root = new Entity;

    Q_INIT_RESOURCE(resources);



    for(auto fixture : photonApp->project()->fixtures()->fixtures())
    {
        FixtureModel *model = new FixtureModel(fixture);

        if(model)
            model->entity()->setParent(root);

        m_models.append(model);
    }

    connect(photonApp->busEvaluator(), &BusEvaluator::evaluationCompleted, this, &VisualizerPanel::dmxUpdated);

    /*
    Entity *cubeEntity = new Entity(root);

    TransformComponent *transform = new TransformComponent;
    transform->setPosition(QVector3D(0,0,0));
    transform->setRotationEuler(QVector3D(0,45,0));
    cubeEntity->addComponent(transform);

    MaterialComponent *material = new MaterialComponent;
    material->setFragmentSource(":/resources/shader/basic_lighting.frag");
    material->setVertexSource(":/resources/shader/basic_lighting.vert");
    cubeEntity->addComponent(material);

    CubeMesh *cube = new CubeMesh;
    cubeEntity->addComponent(cube);

*/


    m_viewport->setRootEntity(root);

}

VisualizerPanel::~VisualizerPanel()
{

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
