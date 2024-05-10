#include "canvasreader.h"
#include "routine/routineevaluationcontext.h"

namespace photon {

keira::NodeInformation CanvasReader::info()
{
    keira::NodeInformation toReturn([](){return new CanvasReader;});
    toReturn.name = "Canvas Reader";
    toReturn.nodeId = "photon.plugin.node.canvas-reader";

    return toReturn;
}

CanvasReader::CanvasReader() : keira::Node("photon.plugin.node.canvas-reader")
{

}

void CanvasReader::createParameters()
{
    m_canvasParam = new CanvasParameter("canvasOutput","Canvas", QImage{},keira::AllowMultipleOutput);
    addParameter(m_canvasParam);


}

void CanvasReader::evaluate(keira::EvaluationContext *t_context) const
{
    /*
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->canvasImage)
    {
        m_canvasParam->setValue(*context->canvasImage);

    }
*/

}

} // namespace photon
