#include <QVBoxLayout>
#include "surfacegraphwidget.h"
#include "view/viewer.h"
#include "view/scene.h"
#include "photoncore.h"
#include "plugin/pluginfactory.h"

namespace photon {

class SurfaceGraphWidget::Impl
{
public:
    SurfaceGraph *graph;
    keira::Viewer *viewer;
    keira::Scene *scene = nullptr;
};

SurfaceGraphWidget::SurfaceGraphWidget(SurfaceGraph *t_graph, QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    m_impl->viewer = new keira::Viewer;
    m_impl->graph = t_graph;

    m_impl->scene = new keira::Scene;
    m_impl->scene->setIsAutoEvaluate(false);
    m_impl->scene->setGraph(t_graph);
    m_impl->scene->setNodeLibrary(photonApp->plugins()->nodeLibrary());


    m_impl->viewer->setScene(m_impl->scene);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(m_impl->viewer);
    setLayout(vLayout);

    setMinimumSize(500,500);

}

SurfaceGraphWidget::~SurfaceGraphWidget()
{
    m_impl->viewer->setScene(nullptr);
    if(m_impl->scene)
        delete m_impl->scene;
    delete m_impl;
}

} // namespace photon
