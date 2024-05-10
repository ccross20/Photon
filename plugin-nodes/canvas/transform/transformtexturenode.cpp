#include <QMatrix4x4>
#include "transformtexturenode.h"
#include "opengl/openglresources.h"

namespace photon {

TransformTextureNode::TransformTextureNode() : BaseRoutineNode("photon.plugin.node.transform-texture") {
    setWidth(300);
}

TransformTextureNode::~TransformTextureNode()
{

}

void TransformTextureNode::createParameters()
{
    m_textureInputParam = new TextureParameter("texInput","Texture In",0);
    addParameter(m_textureInputParam);

    m_positionParam = new Point2DParameter("position","Position", QPointF());
    addParameter(m_positionParam);

    m_scaleParam = new Point2DParameter("scale","Scale", QPointF(1,1));
    addParameter(m_scaleParam);

    m_centerParam = new Point2DParameter("origin","Origin", QPointF());
    addParameter(m_centerParam);

    m_rotationParam = new keira::DecimalParameter("rotation","Rotation", 0.0);
    addParameter(m_rotationParam);

    m_textureOutputParam = new TextureParameter("texOutput","Texture Out",0);
    addParameter(m_textureOutputParam);
}

void TransformTextureNode::initializeContext(QOpenGLContext *t_context, Canvas *t_canvas)
{
    m_texture = new OpenGLTexture;
}

void TransformTextureNode::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);

    TextureData texData = m_textureInputParam->value().value<TextureData>();
    m_texture->resize(context->openglContext, QImage::Format::Format_ARGB32_Premultiplied, texData.size.width(), texData.size.height());

    OpenGLFrameBuffer buffer(m_texture, context->openglContext);
    context->resources->bind(context->openglContext);
    context->resources->clear();

    QPointF pos = m_positionParam->value().toPointF();
    QPointF center = m_centerParam->value().toPointF();
    //QPointF scale = m_scaleParam->value().toPointF();
    double rotation = m_rotationParam->value().toDouble();

    QMatrix4x4 viewportMatrix;
    viewportMatrix.setToIdentity();
    viewportMatrix.ortho(-1,1, -1,1,-1,1);

    QMatrix4x4 camMatrix;
    camMatrix.setToIdentity();
    camMatrix.translate(pos.x()*2.0, pos.y()*2.0);
    camMatrix.translate(center.x()*2.0, center.y()*2.0);
    camMatrix.rotate(rotation,0,0,1);
    camMatrix.translate(-center.x()*2.0, -center.y()*2.0);

    context->resources->bindProjectedShader(texData.handle, viewportMatrix, camMatrix);
    context->resources->drawPlane();

    m_textureOutputParam->setValue(TextureData{m_texture->handle(), texData.size});
}

keira::NodeInformation TransformTextureNode::info()
{
    keira::NodeInformation toReturn([](){return new TransformTextureNode;});
    toReturn.name = "Transform Texture";
    toReturn.nodeId = "photon.plugin.node.transform-texture";

    return toReturn;
}

} // namespace photon
