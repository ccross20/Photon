#ifndef PHOTON_VISIBLELIGHTMATERIAL_H
#define PHOTON_VISIBLELIGHTMATERIAL_H
#include <QImage>
#include "component/materialcomponent.h"
#include "texture.h"

namespace photon {

class VisibleLightMaterial : public MaterialComponent
{
public:
    VisibleLightMaterial();

    void rebuild(QOpenGLContext *context) override;

    void setImage(const QImage &image);
    void drawMaterial(DrawContext *context, Entity *entity, QOpenGLShaderProgram &program) override;

private:
    QImage m_image;
    Texture m_texture;
};

} // namespace photon

#endif // PHOTON_VISIBLELIGHTMATERIAL_H
