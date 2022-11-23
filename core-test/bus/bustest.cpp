#include <QTest>
#include "bustest.h"
#include "graph/bus/busgraph.h"
#include "graph/bus/dmxgeneratematrixnode.h"
#include "graph/bus/dmxwriternode.h"

namespace photon {

BusTest::BusTest(QObject *parent)
    : QObject{parent}
{

}

void BusTest::readDMX()
{
    DMXGenerateMatrixNode *generateNode = new DMXGenerateMatrixNode;
    generateNode->createParameters();


    DMXWriterNode *writerNode = new DMXWriterNode;
    writerNode->createParameters();

    BusGraph *bus = new BusGraph;

    bus->addNode(generateNode);
    bus->addNode(writerNode);

    bus->connectParameters(generateNode->findParameter(DMXGenerateMatrixNode::OutputDMX), writerNode->findParameter(DMXWriterNode::InputDMX));

    keira::EvaluationContext context;
    QBENCHMARK
    {
        generateNode->markDirty();
        bus->evaluate(&context);
        bus->markClean();
    }

    delete bus;
}

} // namespace photon
