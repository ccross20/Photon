#ifndef PHOTON_DISTANCETRANSFORM_H
#define PHOTON_DISTANCETRANSFORM_H
#include "opengl/openglshader.h"
#include "opengl/opengltexture.h"
#include "opengl/openglframebuffer.h"
#include "opengl/openglplane.h"
#include "photon-global.h"


namespace photon {

class PHOTONCORE_EXPORT DistanceTransform
{
public:
    DistanceTransform();

    enum DistanceMode
    {
        Inner,
        Outer,
        Middle
    };

    DistanceTransform(QOpenGLContext *context, OpenGLTexture *tex, OpenGLFrameBuffer *buffer, OpenGLPlane *plane, DistanceMode mode = Outer);

    OpenGLTexture *texture() const;
    OpenGLPingPongTexture *rawTexture(){return &m_tex_dtnn;}

private:
    OpenGLPingPongTexture m_tex_dtnn;
};

} // namespace photon

#endif // PHOTON_DISTANCETRANSFORM_H
