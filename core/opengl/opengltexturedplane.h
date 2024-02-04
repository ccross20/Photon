#ifndef OPENGLTEXTUREDPLANE_H
#define OPENGLTEXTUREDPLANE_H
#include <qopengl.h>
#include <QVector>
#include <QRectF>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include "data/types.h"

class QOpenGLFunctions;

namespace photon {

class OpenGLTexturedPlane
{
public:
    OpenGLTexturedPlane(QOpenGLContext *context, const bounds_d &bounds, bool flipY = true);
    virtual ~OpenGLTexturedPlane();

    double width() const{return m_bounds.width();}
    double height() const{return m_bounds.height();}
    bounds_d bounds() const{return m_bounds;}
    void setBounds(const bounds_d &bounds);

    void create(GLfloat depth = 0.0f);
    void preDraw(QOpenGLContext *context);
    void draw(QOpenGLContext *context = nullptr);
    void postDraw(QOpenGLContext *context);
    void destroy();
    void setImage(const QImage &img);
    void setTexture(int textureId);
    void updateImage(const QImage &raster, const bounds_i &bounds);
    void setMinFormat(int format);
    void setMagFormat(int format);

    uint vao() const{return m_vao;}

private:
    QOpenGLContext *m_context;
    uint m_vbo;
    uint m_ebo;
    uint m_vao;

    GLfloat m_depth = 0;
    int m_texture;
    uint m_textureWidth;
    uint m_textureHeight;
    uint m_minFormat = GL_LINEAR;
    uint m_magFormat = GL_LINEAR;
    QVector<GLfloat> m_vertices;
    bounds_d m_bounds;
    bool m_flipY = true;
};


} // namespace exo

#endif // OPENGLTEXTUREDPLANE_H
