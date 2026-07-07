// Throwaway micro-benchmark for keira graph evaluation cost.
// Measures the per-frame cost of Graph::prepForEvaluation()+evaluate()+markClean()
// for representative topologies, to size the Surfaces feature against the ~22ms
// (44Hz) frame budget. Not part of the shipping test suite.

#include <QCoreApplication>
#include <QElapsedTimer>
#include <cstdio>
#include <cmath>

#include "model/graph.h"
#include "model/node.h"
#include "model/parameter/decimalparameter.h"

using namespace keira;

// Minimal node: out = a + 1. Two decimal params so we exercise value propagation.
class AddNode : public Node
{
public:
    AddNode() : Node("bench.add") {}

    void createParameters() override
    {
        m_a   = new DecimalParameter("a",   "A",   0.0, AllowMultipleInput | AllowSingleInput);
        m_out = new DecimalParameter("out", "Out", 0.0, AllowMultipleOutput);
        addParameter(m_a);
        addParameter(m_out);
    }

    void evaluate(EvaluationContext *) const override
    {
        m_out->setValue(m_a->value().toDouble() + 1.0);
    }

private:
    mutable DecimalParameter *m_a   = nullptr;
    mutable DecimalParameter *m_out = nullptr;
};

static AddNode *makeNode(Graph *g)
{
    auto *n = new AddNode;
    n->createParameters();
    g->addNode(n);
    return n;
}

// Build a single serial chain of `count` nodes: n0.out -> n1.a -> n2.a ...
static Graph *buildChain(int count)
{
    auto *g = new Graph;
    QVector<AddNode*> nodes;
    for (int i = 0; i < count; ++i)
        nodes.append(makeNode(g));
    g->drainCommandQueue();
    for (int i = 1; i < count; ++i)
        g->connectParameters(nodes[i-1]->uniqueId() + "/out", nodes[i]->uniqueId() + "/a");
    g->drainCommandQueue();
    return g;
}

// Build `groups` independent fragments, each a small `depth`-node chain.
// Models a surface with many gizmo wiring fragments running in parallel.
static Graph *buildFan(int groups, int depth)
{
    auto *g = new Graph;
    for (int grp = 0; grp < groups; ++grp)
    {
        QVector<AddNode*> chain;
        for (int d = 0; d < depth; ++d)
            chain.append(makeNode(g));
        g->drainCommandQueue();
        for (int d = 1; d < depth; ++d)
            g->connectParameters(chain[d-1]->uniqueId() + "/out", chain[d]->uniqueId() + "/a");
    }
    g->drainCommandQueue();
    return g;
}

static double timeEvalUs(Graph *g, int frames)
{
    EvaluationContext ctx;
    // warm up
    for (int i = 0; i < 50; ++i) { g->prepForEvaluation(); g->evaluate(&ctx); g->markClean(); }

    QElapsedTimer t; t.start();
    for (int i = 0; i < frames; ++i)
    {
        g->prepForEvaluation();
        g->evaluate(&ctx);
        g->markClean();
    }
    const double totalNs = double(t.nsecsElapsed());
    return totalNs / frames / 1000.0; // us per frame
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const int frames = 2000;
    const double budgetUs = 1000000.0 / 44.0; // ~22727us

    printf("keira eval benchmark  (frame budget %.0f us @ 44Hz)\n\n", budgetUs);
    printf("SERIAL CHAIN (worst-case dependency depth)\n");
    printf("%8s %12s %10s\n", "nodes", "us/frame", "%budget");
    for (int n : {10, 50, 100, 250, 500, 1000, 2000})
    {
        Graph *g = buildChain(n);
        const double us = timeEvalUs(g, frames);
        printf("%8d %12.2f %10.2f\n", n, us, 100.0*us/budgetUs);
        delete g;
    }

    printf("\nPARALLEL FRAGMENTS (surface-like: N gizmos x depth 4)\n");
    printf("%8s %8s %12s %10s\n", "gizmos", "nodes", "us/frame", "%budget");
    for (int groups : {25, 50, 100, 200, 400})
    {
        const int depth = 4;
        Graph *g = buildFan(groups, depth);
        const double us = timeEvalUs(g, frames);
        printf("%8d %8d %12.2f %10.2f\n", groups, groups*depth, us, 100.0*us/budgetUs);
        delete g;
    }
    fflush(stdout);

    return 0;
}
