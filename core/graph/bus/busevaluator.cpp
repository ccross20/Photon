#include <QElapsedTimer>
#include "busevaluator.h"
#include "model/node.h"
#include "data/dmxmatrix.h"
#include "graph/bus/dmxgeneratematrixnode.h"
#include "graph/bus/dmxwriternode.h"
#include "model/parameter/parameter.h"
#include "routine/routineevaluationcontext.h"
#include "photoncore.h"

namespace photon {

class BusEvaluator::Impl
{
public:
    BusGraph *bus = nullptr;
    DMXMatrix matrix;
    QElapsedTimer timer;
    qlonglong seconds = 0;
    double elapsed = 0;
    qlonglong frame = 0;
    int frameSecondCounter = 0;
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

    //QElapsedTimer t;
    //t.start();
    m_impl->matrix = DMXMatrix{2};
    RoutineEvaluationContext context(m_impl->matrix);
    context.frame = m_impl->frame;
    context.globalTime = QDateTime::currentMSecsSinceEpoch()/1000.0;
    context.relativeTime = context.globalTime;
    context.project = photonApp->project();

    //qDebug() << "Begin eval";
    m_impl->bus->findNode("DMX Generator")->findParameter(DMXGenerateMatrixNode::OutputDMX)->setValue(m_impl->matrix);
    m_impl->bus->prepForEvaluation();
    m_impl->bus->evaluate(&context);
    m_impl->bus->markClean();

    qlonglong seconds = std::floor(context.globalTime);

    if(m_impl->seconds != seconds)
    {
        m_impl->seconds = seconds;
        //qDebug() << "Frames per second" << m_impl->frameSecondCounter;
        m_impl->frameSecondCounter = 0;
    }

    //qDebug() << "Graph eval time: " << t.nsecsElapsed();
    m_impl->elapsed = m_impl->timer.elapsed() / 1000.0;

    m_impl->matrix = m_impl->bus->findNode("output")->findParameter(DMXWriterNode::InputDMX)->value().value<DMXMatrix>();

    //qDebug() << m_impl->matrix.value(0,3);

    //qDebug() << context.frame << QDateTime::currentMSecsSinceEpoch();


    emit evaluationCompleted();
    m_impl->timer.restart();
    m_impl->frame += 1;
    m_impl->frameSecondCounter += 1;
}

} // namespace photon
