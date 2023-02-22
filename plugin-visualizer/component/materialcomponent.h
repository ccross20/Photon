#ifndef PHOTON_MATERIALCOMPONENT_H
#define PHOTON_MATERIALCOMPONENT_H

#include <QOpenGLShaderProgram>
#include <QColor>
#include "component/abstractcomponent.h"
#include "texture.h"

namespace photon {

class MaterialComponent : public AbstractComponent
{
public:
    MaterialComponent();
    virtual ~MaterialComponent();

    void setVertexSource(const QString &source);
    void setFragmentSource(const QString &source);

    bool link();
    bool bind();

    void setDiffuseColor(const QColor &color);

    virtual void create(QOpenGLContext *context) override;
    virtual void rebuild(QOpenGLContext *context) override;
    virtual void draw(DrawContext *context) override;
    virtual void destroy(QOpenGLContext *context) override;

    virtual void drawMaterial(DrawContext *context, Entity *entity, QOpenGLShaderProgram &program);

private:

    void populateLights(QOpenGLContext *context);
    QOpenGLShaderProgram m_program;
    QString m_vertexSourcePath;
    QString m_fragmentSourcePath;
    QColor m_diffuse;
    Texture *m_texture;
};

} // namespace photon

#endif // PHOTON_MATERIALCOMPONENT_H
