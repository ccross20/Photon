#include <QPainter>
#include "renderpath.h"

namespace photon {

keira::NodeInformation RenderPath::info()
{
    keira::NodeInformation toReturn([](){return new RenderPath;});
    toReturn.name = "Render Path";
    toReturn.nodeId = "photon.plugin.node.render-path";

    return toReturn;
}

RenderPath::RenderPath() : BaseRoutineNode("photon.plugin.node.render-path")
{

}

RenderPath::~RenderPath()
{

}

void RenderPath::initializeContext(QOpenGLContext *, Canvas *)
{
    m_texture = new OpenGLTexture;
}

void RenderPath::createParameters()
{
    m_pathInputParam = new PathParameter("pathOutput","Path In", QPainterPath{});
    addParameter(m_pathInputParam);

    m_colorParam = new ColorParameter("colorInput","Color", Qt::red);
    addParameter(m_colorParam);

    m_alphaParam = new keira::DecimalParameter("alphaInput","Alpha", 1.0);
    m_alphaParam->setMinimum(0);
    m_alphaParam->setMaximum(1);
    addParameter(m_alphaParam);

    m_sizeParam = new keira::OptionParameter("size","Size", {"64","128","256","512","1024","2048"}, 3);
    addParameter(m_sizeParam);

    m_textureInputParam = new TextureParameter("texInput","Texture In",0);
    addParameter(m_textureInputParam);

    m_textureOutputParam = new TextureParameter("texOutput","Texture Out",0);
    addParameter(m_textureOutputParam);


}

void RenderPath::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);

    int size = 64;
    switch(m_sizeParam->value().toInt())
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

    QImage image = QImage{size,size,QImage::Format_ARGB32_Premultiplied};
    image.fill(Qt::transparent);
    QPainter painter{&image};
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setOpacity(m_alphaParam->value().toDouble());

    QTransform t;
    t.translate(-.5,-.5);
    t.scale(size,size);
    t.translate(.5,.5);

    painter.fillPath(t.map(m_pathInputParam->value().value<QPainterPath>()),m_colorParam->value().value<QColor>());

    m_texture->resize(context->openglContext, &image, image.bits());

    m_textureOutputParam->setValue(TextureData{m_texture->handle(), QSize{size, size}});
}

} // namespace photon
