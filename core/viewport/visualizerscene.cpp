#include <QPainter>
#include <Qt3DCore/qtransform.h>
#include <Qt3DExtras/QDiffuseSpecularMaterial>
#include "visualizerscene.h"

namespace photon {

class GoboTexture : public Qt3DRender::QPaintedTextureImage
{
protected:
     void paint(QPainter *painter)
     {
         painter->fillRect(painter->clipBoundingRect(), Qt::white);
         painter->translate(256,256);
         painter->fillRect(-200,-50,400,100, Qt::cyan);
         painter->fillRect(-50,-200,100,400, Qt::cyan);
     }
};

VisualizerScene::VisualizerScene(Qt3DCore::QEntity *rootEntity)
    : m_rootEntity(rootEntity)
{    
    m_gobo = new GoboTexture;
    m_gobo->setSize(QSize{512,512});
    m_gobo->update();

    Qt3DRender::QTexture2D *texture = new Qt3DRender::QTexture2D;
    texture->addTextureImage(m_gobo);

/*
    Qt3DCore::QEntity *light2Entity = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QDirectionalLight *light2 = new Qt3DRender::QDirectionalLight();

    light2->setColor(Qt::cyan);
    light2->setIntensity(.5f);
    light2Entity->addComponent(light2);
    Qt3DCore::QTransform *lightTransform2 = new Qt3DCore::QTransform();
    lightTransform2->setTranslation(QVector3D(0.0f, 0.0f, -2.0f));
    light2Entity->addComponent(lightTransform2);
*/




    m_torus = new Qt3DExtras::QPlaneMesh();
    m_torus->setHeight(10);
    m_torus->setWidth(10);
    //! [0]

    // TorusMesh Transform
    //! [1]
    Qt3DCore::QTransform *torusTransform = new Qt3DCore::QTransform();
    torusTransform->setScale(2.0f);
    //torusTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0.0f, 1.0f, 0.0f), 25.0f));
    torusTransform->setTranslation(QVector3D(0.0f, 0.0f, 0.0f));
    //! [1]

    //! [2]
    Qt3DExtras::QDiffuseSpecularMaterial *torusMaterial = new Qt3DExtras::QDiffuseSpecularMaterial();
    torusMaterial->setDiffuse(QVariant::fromValue(texture));
    torusMaterial->setShininess(.2f);
    //! [2]

    m_torusEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_torusEntity->addComponent(m_torus);
    m_torusEntity->addComponent(torusMaterial);
    m_torusEntity->addComponent(torusTransform);



    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_light = new Qt3DRender::QPointLight();

    m_light->setColor(Qt::red);
    m_light->setIntensity(100.0f);
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform();
    lightTransform->setTranslation(QVector3D(0.0f, 3.0f, 5.0f));
    lightEntity->addComponent(lightTransform);
    lightEntity->addComponent(m_light);



}

VisualizerScene::~VisualizerScene()
{

}

} // namespace photon
