#include <QPainter>
#include "drawrectangle.h"

namespace photon {

keira::NodeInformation DrawRectangle::info()
{
    keira::NodeInformation toReturn([](){return new DrawRectangle;});
    toReturn.name = "Draw Rectangle";
    toReturn.nodeId = "photon.plugin.node.draw-rectangle";

    return toReturn;
}

DrawRectangle::DrawRectangle() : keira::Node("photon.plugin.node.draw-rectangle")
{

}

void DrawRectangle::createParameters()
{
    m_widthParam = new keira::DecimalParameter("widthInput","Width", .5);
    m_widthParam->setMinimum(0);
    m_widthParam->setMaximum(1);
    addParameter(m_widthParam);

    m_heightParam = new keira::DecimalParameter("heightInput","Height", .5);
    m_heightParam->setMinimum(0);
    m_heightParam->setMaximum(1);
    addParameter(m_heightParam);

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

void DrawRectangle::evaluate(keira::EvaluationContext *t_context) const
{
    QImage image = m_canvasInputParam->value().value<QImage>();
    QPainter painter{&image};

    painter.setOpacity(m_alphaParam->value().toDouble());
    painter.fillRect(0,0, image.width() * m_widthParam->value().toDouble(), image.height() * m_heightParam->value().toDouble(), m_colorParam->value().value<QColor>());

    m_canvasOutputParam->setValue(image);
}

} // namespace photon
