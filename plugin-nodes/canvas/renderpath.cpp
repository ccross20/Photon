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

RenderPath::RenderPath() : keira::Node("photon.plugin.node.render-path")
{

}

void RenderPath::createParameters()
{
    m_pathInputParam = new PathParameter("pathOutput","Path Out", QPainterPath{});
    addParameter(m_pathInputParam);

    m_colorParam = new ColorParameter("colorInput","Color", Qt::red);
    addParameter(m_colorParam);

    m_alphaParam = new keira::DecimalParameter("alphaInput","Alpha", 1.0);
    m_alphaParam->setMinimum(0);
    m_alphaParam->setMaximum(1);
    addParameter(m_alphaParam);

    m_canvasInputParam = new CanvasParameter("canvasInput","Canvas In", QImage{});
    addParameter(m_canvasInputParam);

    m_canvasOutputParam = new CanvasParameter("canvasOutput","Canvas Out", QImage{});
    addParameter(m_canvasOutputParam);


}

void RenderPath::evaluate(keira::EvaluationContext *t_context) const
{
    QImage image = m_canvasInputParam->value().value<QImage>();
    QPainter painter{&image};
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setOpacity(m_alphaParam->value().toDouble());
    painter.fillPath(m_pathInputParam->value().value<QPainterPath>(),m_colorParam->value().value<QColor>());

    m_canvasOutputParam->setValue(image);
}

} // namespace photon
