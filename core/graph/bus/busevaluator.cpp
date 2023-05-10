#include <QElapsedTimer>
#include "busevaluator.h"
#include "model/node.h"
#include "data/dmxmatrix.h"
#include "graph/bus/dmxgeneratematrixnode.h"
#include "graph/bus/dmxwriternode.h"
#include "model/parameter/parameter.h"

namespace photon {

class BusEvaluator::Impl
{
public:
    BusGraph *bus = nullptr;
    DMXMatrix matrix;
    QElapsedTimer timer;
    double elapsed = 0;
};

BusEvaluator::BusEvaluator(QObject *parent)
    : QObject{parent},m_impl(new Impl)
{

}

BusEvaluator::~BusEvaluator()
{
    delete m_impl;
}

void BusEvaluator::setBus(BusGraph *t_bus)
{
    m_impl->bus = t_bus;
}

BusGraph *BusEvaluator::bus()
{
    return m_impl->bus;
}

double BusEvaluator::elapsed() const
{
    return m_impl->elapsed;
}

const DMXMatrix &BusEvaluator::dmxMatrix() const
{
    return m_impl->matrix;
}

void BusEvaluator::evaluate()
{
    if(!m_impl->bus)
        return;

    keira::EvaluationContext context;
    m_impl->bus->findNode("DMX Generator")->findParameter(DMXGenerateMatrixNode::OutputDMX)->setValue(m_impl->matrix);
    m_impl->bus->evaluate(&context);
    m_impl->elapsed = m_impl->timer.elapsed() / 1000.0;

    m_impl->matrix = m_impl->bus->findNode("output")->findParameter(DMXWriterNode::InputDMX)->value().value<DMXMatrix>();

    emit evaluationCompleted();
    m_impl->timer.restart();
}

} // namespace photon
