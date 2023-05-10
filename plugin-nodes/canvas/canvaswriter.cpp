#include "canvaswriter.h"
#include "routine/routineevaluationcontext.h"

namespace photon {

keira::NodeInformation CanvasWriter::info()
{
    keira::NodeInformation toReturn([](){return new CanvasWriter;});
    toReturn.name = "Canvas Writer";
    toReturn.nodeId = "photon.plugin.node.canvas-writer";

    return toReturn;
}

CanvasWriter::CanvasWriter() : keira::Node("photon.plugin.node.canvas-writer")
{

}

void CanvasWriter::createParameters()
{

    m_canvasParam = new CanvasParameter("canvasInput","Canvas", QImage{});
    addParameter(m_canvasParam);


}

void CanvasWriter::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->canvasImage)
    {
        QImage other = m_canvasParam->value().value<QImage>();
        context->canvasImage->swap(other);

    }

}

} // namespace photon
