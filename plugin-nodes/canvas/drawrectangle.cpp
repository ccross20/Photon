#include <QPainter>
#include "drawrectangle.h"
#include "routine/routineevaluationcontext.h"

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

    m_radiusParam = new keira::DecimalParameter("radiusInput","Radius", .25);
    m_radiusParam->setMinimum(0);
    m_radiusParam->setMaximum(1);
    addParameter(m_radiusParam);

    m_pathOutputParam = new PathParameter("pathOutput","Path Out", QPainterPath{});
    addParameter(m_pathOutputParam);

}

void DrawRectangle::evaluate(keira::EvaluationContext *t_context) const
{

    QPainterPath path;

    double w = m_widthParam->value().toDouble();
    double h = m_heightParam->value().toDouble();
    double r = m_radiusParam->value().toDouble();
    r = std::min(std::min(r,w/2.0),h/2.0);

    if(r <= 0.0)
        path.addRect(-w/2.0, -h/2.0, w, h);
    else
        path.addRoundedRect(-w/2.0, -h/2.0, w, h, r,r);

    m_pathOutputParam->setValue(QVariant::fromValue(path));

}

} // namespace photon
