#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include "graphwidget.h"
#include "viewer.h"
#include "nodeeditor.h"
#include "nodeitem.h"
#include "scene.h"
#include "model/node.h"

namespace keira {

GraphWidget::GraphWidget(NodeLibrary *t_library, QWidget *parent)
    : QWidget{parent}
{
    m_viewer = new keira::Viewer(t_library);
    m_editor = new keira::NodeEditor;

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

    QWidget *viewerContainer = new QWidget;
    viewerContainer->setLayout(vLayout);

    connect(m_viewer, &keira::Viewer::subGraphClicked, this, &GraphWidget::subGraphOpened);

    QSplitter *splitter = new QSplitter;
    splitter->addWidget(viewerContainer);
    splitter->addWidget(m_editor);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(splitter);
    setLayout(hLayout);
}

GraphWidget::~GraphWidget()
{
    // QWidget's destructor tears down our widget children (m_viewer, m_editor,
    // m_breadcrumbBar, ...) before ~QObject() gets to plain-QObject children
    // like a Scene parented to us - so if m_scene outlives them even briefly,
    // its own destructor removing/deselecting items fires selectionChanged(),
    // which is still connected to selectionUpdated() and would dereference the
    // already-freed m_editor. Cut the connection first so nothing can fire
    // during teardown, regardless of which child gets destroyed first.
    if(m_scene)
        m_scene->disconnect(this);
}

void GraphWidget::subGraphOpened(Graph *t_graph)
{
    m_editor->setNode(nullptr);
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

    m_editor->setNode(nullptr);
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
        m_editor->setNode(nullptr);
    }
    else
    {
        for(auto item : items)
        {
            NodeItem *nodeItem = dynamic_cast<NodeItem*>(item);
            if(nodeItem)
            {
                m_editor->setNode(nodeItem->node());
                return;
            }
        }
    }
}

} // namespace keira
