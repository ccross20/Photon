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

    NodeItem *itemForNode(Node *) const;

public slots:
    void updateFromNodes();

signals:

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent) override;

private slots:

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
