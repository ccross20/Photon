#include <QOpenGLExtraFunctions>
#include "camera.h"
#include "visiblelightmaterial.h"
#include "component/transformcomponent.h"
#include "entity.h"

namespace photon {

VisibleLightMaterial::VisibleLightMaterial()
{
    setFragmentSource(":/resources/shader/visible_light.frag");
    setVertexSource(":/resources/shader/visible_light.vert");
}

void VisibleLightMaterial::rebuild(QOpenGLContext *t_context)
{
    MaterialComponent::rebuild(t_context);

    if(!m_image.isNull())
    {
        m_texture.bind(t_context);
        m_texture.resize(t_context, m_image, m_image.constBits());
        //m_texture.generateMip();
    }

}

void VisibleLightMaterial::setImage(const QImage &t_image)
{
    m_image = t_image;
    setDirty(Dirty_Rebuild);
}

void VisibleLightMaterial::drawMaterial(DrawContext *t_context, Entity *t_entity, QOpenGLShaderProgram &t_program)
{

    TransformComponent *transform = t_entity->findComponent<TransformComponent*>();
    if(transform)
    {
        t_program.setUniformValue("model", transform->globalMatrix());
    }
    else
    {
        t_program.setUniformValue("model", QMatrix4x4{});
    }

    t_program.setUniformValue("projection", t_context->camera->matrix());
    m_texture.bind(t_context->openGLContext);
    t_program.setUniformValue("tex", 0);
}

} // namespace photon
