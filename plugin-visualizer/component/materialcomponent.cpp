#include "materialcomponent.h"
#include "transformcomponent.h"
#include "abstractmesh.h"
#include "entity.h"
#include "camera.h"

namespace photon {

MaterialComponent::MaterialComponent()
{
    m_diffuse = Qt::red;
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
    if (!m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, m_vertexSourcePath))
        return;
    if (!m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, m_fragmentSourcePath))
        return;

    link();

    AbstractComponent::rebuild(t_context);
    setDirty(Dirty_Draw);
}

void MaterialComponent::draw(DrawContext *t_context)
{
    bind();
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


    m_program.setUniformValue("projection", t_context->camera->matrix());

    m_program.setUniformValue("lightPos", QVector3D(700,120,30));
    m_program.setUniformValue("viewPos", t_context->camera->position());
    m_program.setUniformValue("lightColor", QVector3D(1,1,1));
    m_program.setUniformValue("objectColor", QVector3D(m_diffuse.redF(),m_diffuse.greenF(),m_diffuse.blueF()));
}

void MaterialComponent::destroy(QOpenGLContext *t_context)
{

}


} // namespace photon
