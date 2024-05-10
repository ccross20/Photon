#ifndef PHOTON_OPENGLRESOURCES_H
#define PHOTON_OPENGLRESOURCES_H
#include "photon-global.h"

class QOpenGLContext;

namespace photon {

class PHOTONCORE_EXPORT OpenGLResources
{
public:
    OpenGLResources();
    ~OpenGLResources();

    void init(QOpenGLContext *);
    void destroy(QOpenGLContext *);
    void bind(QOpenGLContext *);

    void drawPlane() const;
    void bindBasicShader(int textureHandle);
    void bindProjectedShader(int textureHandle, const QMatrix4x4 &viewportMatrix, const QMatrix4x4 &cameraMatrix);
    OpenGLShader *basicShader() const;
    OpenGLShader *projectedShader() const;

    void clear(float r = 0.0, float g = 0.0, float b = 0.0, float a = 0.0) const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_OPENGLRESOURCES_H
