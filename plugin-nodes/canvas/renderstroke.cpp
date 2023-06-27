#include <QPainter>
#include "renderstroke.h"

namespace photon {

keira::NodeInformation RenderStroke::info()
{
    keira::NodeInformation toReturn([](){return new RenderStroke;});
    toReturn.name = "Render Stroke";
    toReturn.nodeId = "photon.plugin.node.render-stroke";

    return toReturn;
}

RenderStroke::RenderStroke() : keira::Node("photon.plugin.node.render-stroke")
{

}

void RenderStroke::createParameters()
{
    m_pathInputParam = new PathParameter("pathOutput","Path Out", QPainterPath{});
    addParameter(m_pathInputParam);

    m_colorParam = new ColorParameter("colorInput","Color", Qt::red);
    addParameter(m_colorParam);


    m_thicknessParam = new keira::DecimalParameter("thicknessInput","Thickness", 3.0);
    m_thicknessParam->setMinimum(0);
    m_thicknessParam->setMaximum(100);
    addParameter(m_thicknessParam);

    m_alphaParam = new keira::DecimalParameter("alphaInput","Alpha", 1.0);
    m_alphaParam->setMinimum(0);
    m_alphaParam->setMaximum(1);
    addParameter(m_alphaParam);

    m_canvasInputParam = new CanvasParameter("canvasInput","Canvas In", QImage{});
    addParameter(m_canvasInputParam);

    m_canvasOutputParam = new CanvasParameter("canvasOutput","Canvas Out", QImage{});
    addParameter(m_canvasOutputParam);


}

void RenderStroke::evaluate(keira::EvaluationContext *t_context) const
{
    QImage image = m_canvasInputParam->value().value<QImage>();
    QPainter painter{&image};
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(m_colorParam->value().value<QColor>(), m_thicknessParam->value().toDouble()));
    painter.setOpacity(m_alphaParam->value().toDouble());
    painter.drawPath(m_pathInputParam->value().value<QPainterPath>());

    m_canvasOutputParam->setValue(image);
}

} // namespace photon
