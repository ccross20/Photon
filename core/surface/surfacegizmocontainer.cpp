#include "surfacegizmocontainer.h"
#include "surfacegraph.h"
#include "viewer/surfacegizmocontainerwidget.h"
#include "model/node.h"
#include "graph/bus/dmxwriternode.h"
#include "graph/bus/dmxreadernode.h"
#include "model/parameter/parameter.h"

namespace photon {

class SurfaceGizmoContainer::Impl
{
public:
    Impl();
    ~Impl();

    QVector<SurfaceGizmo*> gizmos;
    SurfaceGraph *graph;
};

SurfaceGizmoContainer::Impl::Impl()
{
    graph = new SurfaceGraph();

    DMXReaderNode *readerNode = new DMXReaderNode;
    readerNode->setName("output");
    readerNode->createParameters();
    readerNode->setPosition(QPointF(0,0));

    DMXWriterNode *writerNode = new DMXWriterNode;
    writerNode->setName("input");
    writerNode->createParameters();
    writerNode->setPosition(QPointF(800,0));


    graph->addNode(readerNode);
    graph->addNode(writerNode);


    graph->connectParameters(readerNode->findParameter(DMXReaderNode::OutputDMX), writerNode->findParameter(DMXWriterNode::InputDMX));

}

SurfaceGizmoContainer::Impl::~Impl()
{
    delete graph;
}

SurfaceGizmoContainer::SurfaceGizmoContainer(QObject *parent):QObject(parent),m_impl(new Impl) {}

SurfaceGizmoContainer::~SurfaceGizmoContainer()
{
    delete m_impl;
}


void SurfaceGizmoContainer::processChannels(ProcessContext &t_context, double t_lastTime)
{
    keira::EvaluationContext context;

    //m_impl->elapsed = m_impl->timer.elapsed() / 1000.0;

    auto inputNode = m_impl->graph->findNode("output");
    auto outputNode = m_impl->graph->findNode("input");

    if(inputNode && outputNode)
    {
        inputNode->findParameter(DMXReaderNode::OutputDMX)->setValue(t_context.dmxMatrix);

        m_impl->graph->evaluate(&context);
        t_context.dmxMatrix = outputNode->findParameter(DMXWriterNode::InputDMX)->value().value<DMXMatrix>();

    }



}

const QVector<SurfaceGizmo*> &SurfaceGizmoContainer::gizmos() const
{
    return m_impl->gizmos;
}

SurfaceGraph *SurfaceGizmoContainer::graph() const
{
    return m_impl->graph;
}

SurfaceGizmoContainerWidget *SurfaceGizmoContainer::createWidget(SurfaceMode mode)
{
    return new SurfaceGizmoContainerWidget(this, mode);
}

void SurfaceGizmoContainer::addGizmo(photon::SurfaceGizmo *t_gizmo)
{
    if(m_impl->gizmos.contains(t_gizmo))
        return;
    emit gizmoWillBeAdded(t_gizmo, m_impl->gizmos.length());
    m_impl->gizmos.append(t_gizmo);
    emit gizmoWasAdded(t_gizmo, m_impl->gizmos.length()-1);
}

void SurfaceGizmoContainer::removeGizmo(photon::SurfaceGizmo *t_gizmo)
{
    if(!m_impl->gizmos.contains(t_gizmo))
        return;

    int index = 0;
    for(auto seq : m_impl->gizmos)
    {
        if(seq == t_gizmo)
            break;
        ++index;
    }

    //m_impl->panels.remove(t_sequence);

    emit gizmoWillBeRemoved(t_gizmo, index);
    m_impl->gizmos.removeOne(t_gizmo);
    emit gizmoWasRemoved(t_gizmo, index);
}

} // namespace photon
