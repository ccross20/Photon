#include "dmxviewerpanel.h"
#include "gui/dmxviewer/dmxviewercells.h"
#include "photoncore.h"
#include "project/project.h"
#include "graph/bus/busgraph.h"
#include "graph/bus/busevaluator.h"
#include "graph/bus/dmxwriternode.h"
#include "model/parameter/parameter.h"
#include "data/dmxmatrix.h"

namespace photon {

class DMXViewerPanel::Impl
{
public:
    DMXViewerCells *cells;
};

DMXViewerPanel::DMXViewerPanel() : Panel("photon.dmx-viewer"),m_impl(new Impl)
{
    m_impl->cells = new DMXViewerCells;
    setPanelWidget(m_impl->cells);
    setName("DMX Viewer");

    connect(photonApp->busEvaluator(), &BusEvaluator::evaluationCompleted, this, &DMXViewerPanel::tick);
}

DMXViewerPanel::~DMXViewerPanel()
{
    delete m_impl;
}

void DMXViewerPanel::tick()
{
    auto outputNode = dynamic_cast<DMXWriterNode*>(photonApp->project()->bus()->findNode("output"));

    if(outputNode)
    {
        auto dmxParam = outputNode->findParameter(DMXWriterNode::InputDMX);
        if(dmxParam)
            m_impl->cells->setData(dmxParam->value().value<DMXMatrix>());
    }

}

} // namespace photon
