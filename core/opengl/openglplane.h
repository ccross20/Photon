#ifndef DECO_OPENGLPLANE_H
#define DECO_OPENGLPLANE_H
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include "data/types.h"
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT OpenGLPlane
{
public:
    OpenGLPlane(QOpenGLContext *context, const bounds_d &bounds = bounds_d{-1,-1,1,1}, bool flipY = true, bool flipX = false);
    ~OpenGLPlane();

    void destroy();

    double width() const{return m_bounds.width();}
    double height() const{return m_bounds.height();}
    bounds_d bounds() const{return m_bounds;}
    void setFlipY(bool value){m_flipY = value;}
    void setBounds(const bounds_d &bounds);
    void setPoints(point_f ptA, point_f ptB, point_f ptC, point_f ptD);

    void bind(QOpenGLContext *context);
    void draw();
    void draw(QOpenGLContext *context);

private:
    uint m_vbo;
    uint m_ebo;
    uint m_vao;

    QOpenGLContext *m_context;
    GLfloat m_depth = 0;
    QVector<GLfloat> m_vertices;
    bounds_d m_bounds;
    bool m_flipY = true;
    bool m_flipX = false;
    bool m_isValid = false;
};

} // namespace exo

#endif // DECO_OPENGLPLANE_H
