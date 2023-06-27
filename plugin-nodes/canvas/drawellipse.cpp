#include <QPainter>
#include "drawellipse.h"
#include "routine/routineevaluationcontext.h"

namespace photon {


keira::NodeInformation DrawEllipse::info()
{
    keira::NodeInformation toReturn([](){return new DrawEllipse;});
    toReturn.name = "Draw Ellipse";
    toReturn.nodeId = "photon.plugin.node.draw-ellipse";

    return toReturn;
}

DrawEllipse::DrawEllipse() : keira::Node("photon.plugin.node.draw-ellipse")
{

}

void DrawEllipse::createParameters()
{
    m_widthParam = new keira::DecimalParameter("widthInput","Width", .5);
    m_widthParam->setMinimum(0);
    m_widthParam->setMaximum(1);
    addParameter(m_widthParam);

    m_heightParam = new keira::DecimalParameter("heightInput","Height", .5);
    m_heightParam->setMinimum(0);
    m_heightParam->setMaximum(1);
    addParameter(m_heightParam);


    m_pathOutputParam = new PathParameter("pathOutput","Path Out", QPainterPath{});
    addParameter(m_pathOutputParam);
}

void DrawEllipse::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);

    QPainterPath path;

    if(context->canvasImage)
        path.addEllipse(QPointF(0,0), context->canvasImage->width() * m_widthParam->value().toDouble(), context->canvasImage->height() * m_heightParam->value().toDouble());

    m_pathOutputParam->setValue(QVariant::fromValue(path));
}

} // namespace photon
