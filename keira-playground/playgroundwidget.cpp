#include <QVBoxLayout>
#include "playgroundwidget.h"
#include "view/viewer.h"
#include "view/scene.h"
#include "model/graphevaluator.h"

namespace keira {

PlaygroundWidget::PlaygroundWidget(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0,0,0,0);

    Viewer *viewer = new Viewer();

    vLayout->addWidget(viewer);
    setLayout(vLayout);

    Scene *scene = new Scene;
    Graph *graph = new Graph;
    scene->setGraph(graph);

    viewer->setScene(scene);

    //scene->graph()->evaluate();

    setMinimumSize(1200,900);
}

} // namespace keira
