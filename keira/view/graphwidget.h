#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include "keira-global.h"

namespace keira {

class Scene;
class Viewer;
class Node;

class KEIRA_EXPORT GraphWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GraphWidget(NodeLibrary *t_library, QWidget *parent = nullptr);
    ~GraphWidget();

    void setScene(Scene *t_scene);
    Scene *scene() const;

signals:
    // The selected node changed (null when the selection is empty or holds no
    // node). keira has no property UI of its own any more - photon-core listens
    // and routes this to the Properties panel.
    void nodeSelected(keira::Node *node);

private slots:
    void selectionUpdated();
    void subGraphOpened(Graph *);
    void graphUpdated(Graph *);
    void gotoParentGraph();

private:
    // Rebuilds the breadcrumb row (m_breadcrumbBar) for t_graph's ancestry -
    // one clickable crumb per ancestor (named for the node that contains it),
    // plus a final plain, non-clickable crumb for t_graph itself.
    void rebuildBreadcrumbs(Graph *t_graph);
    // Common landing point for both the Up button and a breadcrumb click.
    void navigateToGraph(Graph *t_graph);

    Scene *m_scene = nullptr;
    Viewer *m_viewer = nullptr;
    QWidget *m_breadcrumbBar;
    QHBoxLayout *m_breadcrumbLayout;
    QPushButton *m_upButton;
    QPushButton *m_centerButton;
    QPushButton *m_frameButton;
};

} // namespace keira

#endif // GRAPHWIDGET_H
