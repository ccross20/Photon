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

    // Node clipboard - the system clipboard, so nodes can be copied from one
    // graph and pasted into another (or another editor). What's copied is each
    // selected node's saved form plus the connections between the copied
    // nodes; wires to nodes left behind are dropped. Nodes that can't be
    // removed (e.g. a subgraph's Globals) are structural, so they're skipped.
    static const char *NodeClipboardMime;
    bool hasSelectedNodes() const;
    static bool clipboardHasNodes();
    void copySelectedNodes();
    void cutSelectedNodes();
    // Adds the clipboard's nodes centred on t_scenePos with fresh ids and
    // their internal wiring restored, and selects them. Node types the
    // current graph doesn't allow are left out.
    void pasteNodes(const QPointF &scenePos);

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
