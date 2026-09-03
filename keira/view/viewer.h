#ifndef KEIRA_VIEWER_H
#define KEIRA_VIEWER_H

#include <QGraphicsView>
#include "keira-global.h"

namespace keira {

class KEIRA_EXPORT Viewer : public QGraphicsView
{
    Q_OBJECT
public:
    explicit Viewer(NodeLibrary *t_library, QWidget *parent = nullptr);
    virtual ~Viewer();

    Graph *graph() const;
    void zoom(double value);
    void zoom(double value, QPointF pt);

    // Takes over scene wiring so each graph's scroll/zoom is remembered and
    // restored as the user dives into and back out of subgraphs.
    void setScene(QGraphicsScene *scene);

    void deleteSelected();

signals:
    void zoomChanged(double value);
    void subGraphClicked(Graph *);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;


private slots:
    void graphAboutToChange(Graph *oldGraph);
    void graphChanged(Graph *newGraph);

private:
    // Stash the current viewport centre (in scene coords) and zoom for t_graph.
    void saveViewState(Graph *t_graph);
    // Re-apply a previously stashed view state for t_graph, if there is one.
    void restoreViewState(Graph *t_graph);

    // Duplicates a node into this graph and takes over the drag with the copy,
    // so a Ctrl+drag leaves the original (and its wiring) untouched and moves
    // the new one. Returns the copy's item, or null if it couldn't be made.
    NodeItem *startCloneDrag(NodeItem *source, const QPointF &scenePos);

    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // KEIRA_VIEWER_H
