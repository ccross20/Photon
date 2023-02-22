#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include "materialcomponent.h"
#include "transformcomponent.h"
#include "abstractmesh.h"
#include "spotlight.h"
#include "entity.h"
#include "camera.h"

namespace photon {

MaterialComponent::MaterialComponent()
{
    m_diffuse = Qt::red;
    m_texture = new Texture;
}

MaterialComponent::~MaterialComponent()
{

}

void MaterialComponent::setVertexSource(const QString &t_source)
{
    m_vertexSourcePath = t_source;
    setDirty(Dirty_Rebuild);
}

void MaterialComponent::setFragmentSource(const QString &t_source)
{
    m_fragmentSourcePath = t_source;
    setDirty(Dirty_Rebuild);
}

void MaterialComponent::setDiffuseColor(const QColor &color)
{
    m_diffuse = color;
}

bool MaterialComponent::link()
{
    return m_program.link();
}

bool MaterialComponent::bind()
{
    return m_program.bind();
}


void MaterialComponent::create(QOpenGLContext *t_context)
{

}

void MaterialComponent::rebuild(QOpenGLContext *t_context)
{
    Q_ASSERT(m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, m_vertexSourcePath));
    Q_ASSERT(m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, m_fragmentSourcePath));

    /*
    if (!m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, m_vertexSourcePath))
        return;
    if (!m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, m_fragmentSourcePath))
        return;
        */

    link();
    unsigned int testBlock = t_context->extraFunctions()->glGetUniformBlockIndex(m_program.programId(), "LightBlock");
    t_context->extraFunctions()->glUniformBlockBinding(m_program.programId(), testBlock, 0);

    m_texture->loadImage(t_context,QImage(":/resources/cookie.png"));

    AbstractComponent::rebuild(t_context);
    setDirty(Dirty_Draw);
}

void MaterialComponent::draw(DrawContext *t_context)
{
    bind();
    //qDebug() << "Draw" << entity()->name();
    for(auto it = entities().cbegin(); it != entities().cend(); ++it)
    {

        Entity *entity = *it;
        AbstractMesh *mesh = entity->findComponent<AbstractMesh*>();

        if(!mesh)
        {
            qDebug() << "No mesh";
            continue;
        }


        drawMaterial(t_context, entity, m_program);


        mesh->drawMesh(t_context, &m_program);
    }

    setDirty(Dirty_Draw);

}

void MaterialComponent::populateLights(QOpenGLContext *context)
{

}

void MaterialComponent::drawMaterial(DrawContext *t_context, Entity *t_entity, QOpenGLShaderProgram &program)
{
    TransformComponent *transform = t_entity->findComponent<TransformComponent*>();

    if(transform)
    {
        m_program.setUniformValue("model", transform->globalMatrix());
        m_program.setUniformValue("inverseModel", transform->globalMatrix().inverted());
    }
    else
    {
        m_program.setUniformValue("model", QMatrix4x4{});
        m_program.setUniformValue("inverseModel", QMatrix4x4{});
    }


    m_texture->bind(t_context->openGLContext);


    m_program.setUniformValue("projection", t_context->camera->matrix());
    m_program.setUniformValue("viewPos", t_context->camera->position());


    m_program.setUniformValue("material.diffuse", QVector3D(m_diffuse.redF(),m_diffuse.greenF(),m_diffuse.blueF()));
    m_program.setUniformValue("material.specular", QVector3D(0,0,0));

    m_program.setUniformValue("material.shininess", 2.0f);

}

void MaterialComponent::destroy(QOpenGLContext *t_context)
{

}


} // namespace photon
