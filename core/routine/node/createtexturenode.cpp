#include "createtexturenode.h"
#include "model/parameter/optionparameter.h"
#include "graph/parameter/colorparameter.h"
#include "graph/parameter/textureparameter.h"
#include "routine/routineevaluationcontext.h"

namespace photon {

const QByteArray CreateTextureNode::SizeInput = "sizeInput";
const QByteArray CreateTextureNode::ColorInput = "colorInput";
const QByteArray CreateTextureNode::TransparentInput = "transparentInput";

class CreateTextureNode::Impl
{
public:
    keira::OptionParameter *sizeParam;
    ColorParameter *colorParam;
    TextureParameter *textureParam;
    OpenGLTexture *texture;
};

keira::NodeInformation CreateTextureNode::info()
{
    keira::NodeInformation toReturn([](){return new CreateTextureNode;});
    toReturn.name = "Create Texture";
    toReturn.nodeId = "photon.routine.create-texture";

    return toReturn;
}

CreateTextureNode::CreateTextureNode() : BaseRoutineNode("photon.routine.create-texture"),m_impl(new Impl)
{
    setName("Create Texture");
}

CreateTextureNode::~CreateTextureNode()
{
    delete m_impl;
}

void CreateTextureNode::initializeContext(QOpenGLContext *, Canvas *)
{
    m_impl->texture = new OpenGLTexture;
}

void CreateTextureNode::createParameters()
{
    m_impl->sizeParam = new keira::OptionParameter(SizeInput,"Size", {"64","128","256","512","1024","2048"}, 3);

    addParameter(m_impl->sizeParam);

    m_impl->colorParam = new ColorParameter(ColorInput, "Color",Qt::black);
    addParameter(m_impl->colorParam);

    m_impl->textureParam = new TextureParameter("texOutput","Texture",0);
    addParameter(m_impl->textureParam);
}

void CreateTextureNode::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);

    int size = 64;

    QColor color = m_impl->colorParam->value().value<QColor>();


    switch(context->channelValues[SizeInput].toInt())
    {
        case 0:
            size = 64;
            break;
        case 1:
            size = 128;
            break;
        case 2:
            size = 256;
            break;
        case 3:
            size = 512;
            break;
        case 4:
            size = 1024;
            break;
        case 5:
            size = 2048;
            break;
    }

    if(m_impl->texture->width() != size)
        m_impl->texture->resize(context->openglContext, QImage::Format::Format_ARGB32_Premultiplied, size, size);

    auto f = context->openglContext->functions();

    OpenGLFrameBuffer buffer(m_impl->texture, context->openglContext);

    f->glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    f->glClear(GL_COLOR_BUFFER_BIT);

    context->frameBuffer->bind();

    m_impl->textureParam->setValue(TextureData{m_impl->texture->handle(), QSize{size,size}});
}

} // namespace photon
