#include <QHBoxLayout>
#include <QVBoxLayout>
#include "graphwidget.h"
#include "viewer.h"
#include "nodeitem.h"
#include "scene.h"
#include "model/node.h"

namespace keira {

GraphWidget::GraphWidget(NodeLibrary *t_library, QWidget *parent)
    : QWidget{parent}
{
    m_viewer = new keira::Viewer(t_library);

    QVBoxLayout *vLayout = new QVBoxLayout;

    m_breadcrumbBar = new QWidget;
    m_breadcrumbLayout = new QHBoxLayout(m_breadcrumbBar);
    m_breadcrumbLayout->setContentsMargins(0, 0, 0, 0);
    m_breadcrumbLayout->setSpacing(4);

    m_upButton = new QPushButton("Up");
    connect(m_upButton, &QPushButton::clicked,this, &GraphWidget::gotoParentGraph);

    m_centerButton = new QPushButton("Center");
    connect(m_centerButton, &QPushButton::clicked, m_viewer, &Viewer::centerOnAllNodes);

    m_frameButton = new QPushButton("Frame");
    connect(m_frameButton, &QPushButton::clicked, m_viewer, &Viewer::frameAllNodes);

    QHBoxLayout *navLayout = new QHBoxLayout;
    navLayout->addWidget(m_breadcrumbBar);
    navLayout->addWidget(m_centerButton);
    navLayout->addWidget(m_frameButton);
    navLayout->addWidget(m_upButton);

    vLayout->addLayout(navLayout);
    vLayout->addWidget(m_viewer);

    connect(m_viewer, &keira::Viewer::subGraphClicked, this, &GraphWidget::subGraphOpened);

    // The node's parameters used to live in a splitter pane here; they are now
    // shown in the app's Properties panel, so the graph gets the full width.
    setLayout(vLayout);
}

GraphWidget::~GraphWidget()
{
    // QWidget's destructor tears down our widget children (m_viewer,
    // m_breadcrumbBar, ...) before ~QObject() gets to plain-QObject children
    // like a Scene parented to us - so if m_scene outlives them even briefly,
    // its own destructor removing/deselecting items fires selectionChanged(),
    // which is still connected to selectionUpdated() and would emit
    // nodeSelected() out of a half-destroyed widget. Cut the connection first
    // so nothing can fire during teardown, whichever child goes first.
    if(m_scene)
        m_scene->disconnect(this);
}

void GraphWidget::subGraphOpened(Graph *t_graph)
{
    emit nodeSelected(nullptr);
    m_scene->setGraph(t_graph);
}

void GraphWidget::setScene(Scene *t_scene)
{
    if(m_scene == t_scene)
        return;

    if(m_scene)
    {
        disconnect(m_scene, &QGraphicsScene::selectionChanged, this, &GraphWidget::selectionUpdated);
    }

    m_scene = t_scene;
    m_viewer->setScene(m_scene);

    if(m_scene && m_scene->graph())
        rebuildBreadcrumbs(m_scene->graph());

    connect(m_scene, &QGraphicsScene::selectionChanged, this, &GraphWidget::selectionUpdated);
    connect(m_scene, &Scene::graphUpdated, this, &GraphWidget::graphUpdated);
}

Scene *GraphWidget::scene() const
{
    return m_scene;
}

void GraphWidget::navigateToGraph(Graph *t_graph)
{
    if(!t_graph || !m_scene || t_graph == m_scene->graph())
        return;

    emit nodeSelected(nullptr);
    m_scene->setGraph(t_graph);
}

void GraphWidget::gotoParentGraph()
{
    if(m_scene->graph()->parentNode())
        navigateToGraph(m_scene->graph()->parentNode()->graph());
}

void GraphWidget::graphUpdated(Graph *t_graph){
    rebuildBreadcrumbs(t_graph);
}

void GraphWidget::rebuildBreadcrumbs(Graph *t_graph)
{
    QLayoutItem *item;
    while((item = m_breadcrumbLayout->takeAt(0)) != nullptr)
    {
        delete item->widget();
        delete item;
    }

    if(!t_graph)
        return;

    // Root first, t_graph last. Every entry but the last is clickable; the
    // last is shown plain since it's already the graph on screen. Everything
    // after the root is labelled by the node that contains it (its own name,
    // renameable by the user) rather than the graph's own fixed name (see
    // Graph::familyName()) - "Subgraph" told the user nothing about which
    // node they were actually looking inside.
    const QVector<Graph*> chain = t_graph->ancestryChain();
    for(int i = 0; i < chain.size(); ++i)
    {
        Graph *g = chain[i];
        const bool isRoot = (i == 0);
        const bool isCurrent = (i == chain.size() - 1);
        const QString label = (isRoot || !g->parentNode()) ? g->name() : g->parentNode()->name();

        if(isCurrent)
        {
            QLabel *current = new QLabel(label);
            current->setStyleSheet("color: #f0f0f0; font-weight: bold;");
            m_breadcrumbLayout->addWidget(current);
        }
        else
        {
            QPushButton *crumb = new QPushButton(label);
            crumb->setFlat(true);
            crumb->setCursor(Qt::PointingHandCursor);
            crumb->setStyleSheet(
                "QPushButton { border: none; background: transparent; padding: 0px; "
                "color: #8fc7ff; text-decoration: underline; }"
                "QPushButton:hover { color: #ffffff; }");
            connect(crumb, &QPushButton::clicked, this, [this, g](){ navigateToGraph(g); });
            m_breadcrumbLayout->addWidget(crumb);

            QLabel *sep = new QLabel(">");
            sep->setStyleSheet("color: #777777;");
            m_breadcrumbLayout->addWidget(sep);
        }
    }
}

void GraphWidget::selectionUpdated()
{
    auto items = m_scene->selectedItems();
    if(items.isEmpty())
    {
        emit nodeSelected(nullptr);
        return;
    }

    for(auto item : items)
    {
        NodeItem *nodeItem = dynamic_cast<NodeItem*>(item);
        if(nodeItem)
        {
            emit nodeSelected(nodeItem->node());
            return;
        }
    }

    // A selection that holds only wires or comments edits nothing.
    emit nodeSelected(nullptr);
}

} // namespace keira
