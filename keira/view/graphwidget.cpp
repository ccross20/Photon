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
    m_navigationLabel = new QLabel;
    m_upButton = new QPushButton("Up");
    connect(m_upButton, &QPushButton::clicked,this, &GraphWidget::gotoParentGraph);

    QHBoxLayout *navLayout = new QHBoxLayout;
    navLayout->addWidget(m_navigationLabel);
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
        m_navigationLabel->setText(m_scene->graph()->familyName());

    connect(m_scene, &QGraphicsScene::selectionChanged, this, &GraphWidget::selectionUpdated);
    connect(m_scene, &Scene::graphUpdated, this, &GraphWidget::graphUpdated);
}

Scene *GraphWidget::scene() const
{
    return m_scene;
}

void GraphWidget::gotoParentGraph()
{
    m_editor->setNode(nullptr);

    if(m_scene->graph()->parentNode())
        m_scene->setGraph(m_scene->graph()->parentNode()->graph());
}

void GraphWidget::graphUpdated(Graph *t_graph){
    m_navigationLabel->setText(t_graph->familyName());
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
