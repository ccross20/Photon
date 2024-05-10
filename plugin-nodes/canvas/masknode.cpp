#include "masknode.h"
#include "opengl/openglshader.h"
#include "opengl/openglplane.h"
#include "opengl/opengltexture.h"
#include "pixel/canvas.h"

namespace photon {

MaskNode::MaskNode() : BaseRoutineNode("photon.plugin.node.mask") {

}

void MaskNode::createParameters()
{
    m_textureAParam = new TextureParameter("texInputA","Texture A",0);
    addParameter(m_textureAParam);
    m_textureBParam = new TextureParameter("texInputB","Texture B",0);
    addParameter(m_textureBParam);

    m_modeParam = new keira::OptionParameter("mode","Mode", {"A - B","B - A","A - B Inverse","B - A Inverse"}, 0);
    addParameter(m_modeParam);

    m_textureOutputParam = new TextureParameter("texOutput","Texture Out",0);
    addParameter(m_textureOutputParam);
}

void MaskNode::initializeContext(QOpenGLContext *t_context, Canvas *t_canvas)
{
    m_texture = new OpenGLTexture;
    m_texture->resize(t_context, QImage::Format::Format_ARGB32_Premultiplied, t_canvas->width(), t_canvas->height());
    m_plane = new OpenGLPlane(t_context, bounds_d{-1,-1,1,1}, false);
    m_maskShader = new OpenGLShader(t_context, ":/resources/shader/BasicTextureVertex.vert",
                                ":/nodes-resources/shader/texture/mask.frag");
    m_invertShader = new OpenGLShader(t_context, ":/resources/shader/BasicTextureVertex.vert",
                                    ":/nodes-resources/shader/texture/inverted_mask.frag");
    m_outputShader = new OpenGLShader(t_context, ":/resources/shader/BasicTextureVertex.vert",
                                     ":/resources/shader/texture.frag");
}

void MaskNode::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);

    TextureData texA = m_textureAParam->value().value<TextureData>();
    TextureData texB = m_textureBParam->value().value<TextureData>();

    QSize largestSize = texA.size.expandedTo(texB.size);
    m_texture->resize(context->openglContext, QImage::Format::Format_ARGB32_Premultiplied, largestSize.width(), largestSize.height());

    OpenGLFrameBuffer buffer(m_texture, context->openglContext);
    context->openglContext->functions()->glClearColor(.0f,.0f,.0f,.0f);
    context->openglContext->functions()->glClear(GL_COLOR_BUFFER_BIT);

    switch(m_modeParam->value().toInt())
    {
    case 0:
        m_maskShader->bind(context->openglContext);
        m_maskShader->setTexture("inputA",texA.handle);
        m_maskShader->setTexture("inputB",texB.handle);
        break;
    case 1:
        m_maskShader->bind(context->openglContext);
        m_maskShader->setTexture("inputB",texA.handle);
        m_maskShader->setTexture("inputA",texB.handle);
        break;
    case 2:
        m_invertShader->bind(context->openglContext);
        m_invertShader->setTexture("inputA",texA.handle);
        m_invertShader->setTexture("inputB",texB.handle);
        break;
    case 3:
        m_invertShader->bind(context->openglContext);
        m_invertShader->setTexture("inputB",texA.handle);
        m_invertShader->setTexture("inputA",texB.handle);
        break;
    }

    m_plane->draw();

    m_textureOutputParam->setValue(TextureData{m_texture->handle(), largestSize});


}

keira::NodeInformation MaskNode::info()
{
    keira::NodeInformation toReturn([](){return new MaskNode;});
    toReturn.name = "Mask";
    toReturn.nodeId = "photon.plugin.node.mask";

    return toReturn;
}


} // namespace photon
