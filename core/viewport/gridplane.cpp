#include "gridplane.h"
#include <Qt3DRender>
#include <Qt3DExtras>

using namespace Qt3DCore;
using namespace Qt3DRender;
using namespace Qt3DExtras;

namespace photon {

class GridPlane::Impl
{

};

GridPlane::GridPlane(Qt3DCore::QNode *parent) : Qt3DCore::QEntity(parent), m_impl(new Impl)
{
    QMaterial *material1 = new QMaterial();

    // Create effect, technique, render pass and shader
    QEffect *effect = new QEffect();
    QTechnique *gl3Technique = new QTechnique();
    QRenderPass *gl3Pass = new QRenderPass();
    QShaderProgram *glShader = new QShaderProgram();

    QFile vertexFile("C:\\Projects\\photon\\src\\application\\res\\shader\\example.vert");
    vertexFile.open(QIODevice::ReadOnly);
    glShader->setShaderCode(Qt3DRender::QShaderProgram::Vertex, vertexFile.readAll());
    vertexFile.close();

    QFile fragFile("C:\\Projects\\photon\\src\\application\\res\\shader\\example.frag");
    fragFile.open(QIODevice::ReadOnly);
    glShader->setShaderCode(Qt3DRender::QShaderProgram::Fragment, fragFile.readAll());
    fragFile.close();



    // Set the shader on the render pass
    gl3Pass->setShaderProgram(glShader);

    // Add the pass to the technique
    gl3Technique->addRenderPass(gl3Pass);

    // Set the targeted GL version for the technique
    gl3Technique->graphicsApiFilter()->setApi(QGraphicsApiFilter::OpenGL);
    gl3Technique->graphicsApiFilter()->setMajorVersion(3);
    gl3Technique->graphicsApiFilter()->setMinorVersion(1);
    gl3Technique->graphicsApiFilter()->setProfile(QGraphicsApiFilter::CoreProfile);


    QFilterKey *filterKey = new QFilterKey;
    filterKey->setParent(this);
    filterKey->setName(QStringLiteral("renderingStyle"));
    filterKey->setValue(QStringLiteral("forward"));
    gl3Technique->addFilterKey(filterKey);



    QFilterKey *blendKey = new QFilterKey;
    blendKey->setParent(this);
    blendKey->setName(QStringLiteral("renderingStyle"));
    blendKey->setValue(QStringLiteral("forward"));
    gl3Technique->addFilterKey(blendKey);

    QRenderPass *renderPass = new QRenderPass();

    QBlendEquation *blend = new QBlendEquation;
    blend->setBlendFunction(QBlendEquation::BlendFunction::Add);

    QBlendEquationArguments *args = new QBlendEquationArguments;
    args->setSourceRgb(QBlendEquationArguments::SourceAlpha);
    args->setDestinationRgb(QBlendEquationArguments::OneMinusSourceAlpha);
    args->setSourceAlpha(QBlendEquationArguments::One);
    args->setDestinationAlpha(QBlendEquationArguments::Zero);
    renderPass->addRenderState(blend);
    renderPass->addRenderState(args);

    // Create a alpha coverage render state
    QAlphaCoverage *alphaCoverage = new QAlphaCoverage();
    QMultiSampleAntiAliasing *multiSampleAntialiasing = new QMultiSampleAntiAliasing();

    // Add the render states to the render pass
    renderPass->addRenderState(alphaCoverage);
    renderPass->addRenderState(multiSampleAntialiasing);

    gl3Technique->addRenderPass(renderPass);

    // Add the technique to the effect
    effect->addTechnique(gl3Technique);

    // Set the effect on the materials
    material1->setEffect(effect);

    // Set different parameters on the materials
    const QString parameterName = QStringLiteral("color");
    //material1->addParameter(new QParameter(parameterName, QColor::fromRgbF(0.0f, 1.0f, 0.0f, 1.0f)));

    Qt3DExtras::QPlaneMesh *mesh = new Qt3DExtras::QPlaneMesh();
    mesh->setWidth(1000);
    mesh->setHeight(1000);

    addComponent(mesh);

    Qt3DExtras::QPhongMaterial *torusMaterial = new Qt3DExtras::QPhongMaterial();
    torusMaterial->setDiffuse(QColor(QRgb(0xbeb32b)));

    addComponent(material1);

    Qt3DCore::QTransform *trans = new Qt3DCore::QTransform();
    addComponent(trans);
}

GridPlane::~GridPlane()
{
    delete m_impl;
}

void GridPlane::init()
{

}

} // namespace photon
