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


}

} // namespace photon
