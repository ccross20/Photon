#include "evaluationtest.h"
#include "qtestcase.h"
#include "model/graph.h"

namespace keira {

EvaluationTest::EvaluationTest(QObject *parent)
    : QObject{parent}
{

}

void EvaluationTest::simpleEval() const
{
    /*
    Graph *graph = new Graph;

    ArithmeticNode *node1 = new ArithmeticNode;
    node1->setName("node1");
    node1->createParameters();
    ArithmeticNode *node2 = new ArithmeticNode;
    node2->setName("node2");
    node2->createParameters();

    graph->addNode(node2);
    graph->addNode(node1);

    graph->connectParameters("node1/output", "node2/a");

    node1->setValue("a", 10);
    node1->setValue("b", 5);

    node2->setValue("b", 20);

    keira::EvaluationContext context;

    graph->evaluate(&context);
    graph->markClean();

    QCOMPARE(node2->value("output").toInt(), 35);


    node2->setValue("b", 10);
    graph->evaluate(&context);
    graph->markClean();

    //qDebug() << node2->value("output").toInt();
    QCOMPARE(node2->value("output").toInt(), 25);


    delete graph;
    */
}

} // namespace keira
