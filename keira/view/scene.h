#ifndef KEIRA_SCENE_H
#define KEIRA_SCENE_H

#include <QGraphicsScene>
#include "keira-global.h"
#include "model/graph.h"

namespace keira {

class KEIRA_EXPORT Scene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit Scene(QObject *parent = nullptr);
    ~Scene();

    void setIsAutoEvaluate(bool);
    bool isAutoEvaluate() const;
    void setGraph(Graph *);
    Graph *graph() const;
    void setNodeLibrary(NodeLibrary *);

    // Lets the host handle drag-and-drop from outside the graph (e.g. its own
    // asset browser) without keira needing to know what's being dragged - see
    // ExternalDropInterpreter in keira-global.h.
    void setExternalDropInterpreter(ExternalDropInterpreter);

    NodeItem *itemForNode(Node *) const;

public slots:
    void updateFromNodes();

signals:
    void graphUpdated(Graph*);
    // Emitted at the very start of setGraph(), while the outgoing graph is still
    // the one on screen - so a view can stash its scroll/zoom for that graph
    // before the scene is rebuilt for the new one.
    void graphAboutToChange(Graph *oldGraph);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent) override;
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;

private slots:

    void rebuildScene();
    void nodePortsChanged(keira::Node *);
    void nodeWasAdded(keira::Node *);
    void nodeWasRemoved(keira::Node *);
    void nodePositionUpdated(keira::Node *);
    void parametersWereConnected(keira::Parameter *t_out, keira::Parameter *t_in);
    void parametersWereDisconnected(keira::Parameter *t_out, keira::Parameter *t_in);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // KEIRA_SCENE_H
