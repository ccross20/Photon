#include "canvaswriternode.h"
#include "graph/parameter/textureparameter.h"
#include "opengl/openglshader.h"
#include "opengl/openglplane.h"
#include "routine/routineevaluationcontext.h"
#include "opengl/openglresources.h"

namespace photon {

const QByteArray CanvasWriterNode::TextureInput = "textureInput";

class CanvasWriterNode::Impl
{
public:
    TextureParameter *textureParam;
    OpenGLPlane *plane;
    OpenGLShader *shader;
};

keira::NodeInformation CanvasWriterNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasWriterNode;});
    toReturn.name = "Canvas Writer";
    toReturn.nodeId = "photon.routine.texture-writer";

    return toReturn;
}

CanvasWriterNode::CanvasWriterNode(): BaseRoutineNode("photon.routine.texture-writer"),m_impl(new Impl)
{
    setName("Texture Writer");
    setIsAlwaysDirty(true);
}

CanvasWriterNode::~CanvasWriterNode()
{
    delete m_impl;
}

void CanvasWriterNode::initializeContext(QOpenGLContext *t_context, Canvas *)
{
    m_impl->plane = new OpenGLPlane(t_context, bounds_d{-1,-1,1,1}, false);
    m_impl->shader = new OpenGLShader(t_context, ":/resources/shader/BasicTextureVertex.vert",
                                ":/resources/shader/texture.frag");
    m_impl->shader->bind(t_context);
}

void CanvasWriterNode::createParameters()
{
    m_impl->textureParam = new TextureParameter(TextureInput,"Texture", 0);

    addParameter(m_impl->textureParam);
}

void CanvasWriterNode::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);

    context->frameBuffer->bind();
    context->resources->bindBasicShader(m_impl->textureParam->value().value<TextureData>().handle);
    context->resources->drawPlane();
}

} // namespace photon
