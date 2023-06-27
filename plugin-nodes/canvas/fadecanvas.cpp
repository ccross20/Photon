#include <QPainter>
#include "fadecanvas.h"
#include "routine/routineevaluationcontext.h"

namespace photon {


keira::NodeInformation FadeCanvas::info()
{
    keira::NodeInformation toReturn([](){return new FadeCanvas;});
    toReturn.name = "Fade Canvas";
    toReturn.nodeId = "photon.plugin.node.fade-canvas";

    return toReturn;
}

FadeCanvas::FadeCanvas() : keira::Node("photon.plugin.node.fade-canvas")
{

}

void FadeCanvas::createParameters()
{

    m_delayParam = new keira::IntegerParameter("delayInput","Frames", 50);
    m_delayParam->setMinimum(0);
    m_delayParam->setMaximum(255);
    addParameter(m_delayParam);

    m_canvasInputParam = new CanvasParameter("canvasInput","Canvas In", QImage{});
    addParameter(m_canvasInputParam);

    m_canvasOutputParam = new CanvasParameter("canvasOutput","Canvas Out", QImage{});
    addParameter(m_canvasOutputParam);


}

void FadeCanvas::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->canvasImage)
    {
        QImage image = m_canvasInputParam->value().value<QImage>();

        if(context->previousCanvasImage && !context->previousCanvasImage->isNull())
        {
            QPainter painter{&image};

            painter.setOpacity(1.0 - (m_delayParam->value().toInt()/255.0));
            painter.drawImage(QPoint(), *context->previousCanvasImage);
        }

        m_canvasOutputParam->setValue(image);

    }



}

} // namespace photon
