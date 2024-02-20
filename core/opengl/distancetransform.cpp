#include "distancetransform.h"

namespace photon {

DistanceTransform::DistanceTransform(QOpenGLContext *context, OpenGLTexture *tex, OpenGLFrameBuffer *buffer, OpenGLPlane *plane, DistanceMode mode)
{

    int w = buffer->width();
    int h = buffer->height();

    //m_tex_dtnn.resize(context, GL_RG16UI, w, h, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, 2, 2);
    m_tex_dtnn.resize(context, GL_RGBA32F, w, h, GL_BGRA, GL_FLOAT, 4, 1);

    //m_tex_dtnn.resize(context, GL_RG8I, w, h, GL_RG_INTEGER, GL_UNSIGNED_BYTE, 2, 1);
    //m_tex_dtnn.resize(context, PixelFormatRGBA16,w,h);

    //OpenGLShader shaderInit(context, ":/opengl_resources/shader/BasicTextureVertex.vert", ":/opengl_resources/shader/distancetransform_init.frag");
    //OpenGLShader shaderDtnn(context, ":/opengl_resources/shader/BasicTextureVertex.vert", ":/opengl_resources/shader/distancetransform_dtnn.frag");


    OpenGLShader shaderInit(context, ":/resources/shader/BasicTextureVertex.vert", ":/resources/shader/distance/distance_init_fix.frag");
    OpenGLShader shaderDtnn(context, ":/resources/shader/BasicTextureVertex.vert", ":/resources/shader/distance/distance_dtnn_fix.frag");

    QColor c;
    bool invert = false;

    if(tex->channelCount() < 4)
    {
        switch (mode) {
            case Inner: //inside
            c = Qt::black;
            invert = true;
            break;

        case Outer: // outside
            c = Qt::black;
            invert = false;
            break;

        case Middle: // middle
            c = Qt::white;
            invert = true;
            break;
        }
    }
    else
    {
        switch (mode) {
            case Inner: //inside
            c = QColor(0,0,0,0);
            invert = true;
            break;

        case Outer: // outside
            c = QColor(0,0,0,0);
            invert = false;
            break;

        case Middle: // middle
            c = QColor(0,0,0,255);
            invert = true;
            break;
        }
    }




    buffer->setTexture(m_tex_dtnn.destination());


    //tex->bind();
    shaderInit.bind(context);
    shaderInit.setTexture("tex_mask",tex->handle());
    shaderInit.setColor("FG_mask", c);
    shaderInit.setInt("FG_invert", invert ? 0 : 1);
    plane->draw();

    m_tex_dtnn.swap();


    int passes = std::ceil(log(std::max(w, h))/std::log(2)) - 1;


    for(int jump = 1 << passes; jump > 0; jump >>= 1){
        buffer->setTexture(m_tex_dtnn.destination());
        //m_tex_dtnn.source()->bind();
        shaderDtnn.bind(context);
        shaderDtnn.setTexture("tex_dtnn", m_tex_dtnn.source()->handle());
        shaderDtnn.setInt2("wh", w, h);
        shaderDtnn.setInt3("jump", -jump, 0, jump);

        plane->draw();

        m_tex_dtnn.swap();
    }


    //m_tex_dtnn.swap();
}

OpenGLTexture *DistanceTransform::texture() const
{
    return m_tex_dtnn.source();
}

} // namespace deco
