#include "busevaluator.h"
#include "model/graphevaluator.h"
#include "model/node.h"
#include "data/dmxmatrix.h"
#include "graph/bus/dmxgeneratematrixnode.h"
#include "graph/bus/dmxwriternode.h"
#include "model/parameter/parameter.h"
#include "routine/routineevaluationcontext.h"
#include "photoncore.h"
#include <QMutex>
#include <QMutexLocker>

namespace photon {

class BusEvaluator::Impl
{
public:
    BusGraph *bus = nullptr;
    DMXMatrix writeMatrix;
    DMXMatrix readMatrix;
    mutable QMutex matrixMutex;
    keira::GraphEvaluator *graphEvaluator = nullptr;
    qlonglong frame = 0;
};

BusEvaluator::BusEvaluator(QObject *parent)
    : QObject{parent}, m_impl(new Impl)
{
    // Context factory — called on the eval thread at the start of each frame.
    // Creates a RoutineEvaluationContext referencing the write-side DMX buffer.
    auto factory = [this](const keira::FrameTime &t) -> keira::EvaluationContext* {
        m_impl->writeMatrix = DMXMatrix{2};

        if (m_impl->bus) {
            auto *genNode = m_impl->bus->findNode("DMX Generator");
            if (genNode) {
                auto *param = genNode->findParameter(DMXGenerateMatrixNode::OutputDMX);
                if (param)
                    param->setValue(m_impl->writeMatrix);
            }
        }

        m_impl->frame++;
        auto *ctx = new RoutineEvaluationContext(m_impl->writeMatrix);
        ctx->globalTime   = t.elapsed;
        ctx->relativeTime = t.elapsed;
        ctx->project      = photonApp->project();
        ctx->frame        = m_impl->frame;
        return ctx;
    };

    // Post-eval callback — called on the eval thread after the graph finishes.
    // Reads the final DMX output and swaps it into the read buffer under the mutex
    // so the main thread can safely call dmxMatrix() at any time.
    auto postEval = [this]() {
        if (!m_impl->bus)
            return;
        auto *outputNode = m_impl->bus->findNode("output");
        if (!outputNode)
            return;
        auto *param = outputNode->findParameter(DMXWriterNode::InputDMX);
        if (!param)
            return;
        QMutexLocker lock(&m_impl->matrixMutex);
        m_impl->readMatrix = param->value().value<DMXMatrix>();
    };

    // No Qt parent — lifetime is managed manually so we can guarantee
    // the thread stops before m_impl is destroyed (see ~BusEvaluator).
    m_impl->graphEvaluator = new keira::GraphEvaluator(
        nullptr, std::move(factory), std::move(postEval));

    connect(m_impl->graphEvaluator, &keira::GraphEvaluator::evaluationComplete,
            this, &BusEvaluator::onFrameComplete);
}

BusEvaluator::~BusEvaluator()
{
    // Stop the eval thread before Impl is destroyed so the factory/callback
    // lambdas (which capture this) cannot fire after Impl is gone.
    delete m_impl->graphEvaluator;
    m_impl->graphEvaluator = nullptr;
    delete m_impl;
}

void BusEvaluator::setBus(BusGraph *t_bus)
{
    m_impl->bus = t_bus;
    m_impl->graphEvaluator->setGraph(t_bus);
}

BusGraph *BusEvaluator::bus()
{
    return m_impl->bus;
}

DMXMatrix BusEvaluator::dmxMatrix() const
{
    QMutexLocker lock(&m_impl->matrixMutex);
    return m_impl->readMatrix;
}

keira::GraphEvaluator *BusEvaluator::graphEvaluator() const
{
    return m_impl->graphEvaluator;
}

void BusEvaluator::setBpm(double bpm)
{
    m_impl->graphEvaluator->setBpm(bpm);
}

void BusEvaluator::tap()
{
    m_impl->graphEvaluator->tap();
}

void BusEvaluator::onFrameComplete()
{
    emit evaluationCompleted();
}

} // namespace photon
