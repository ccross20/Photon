#ifndef PHOTON_GIZMOGROUP_H
#define PHOTON_GIZMOGROUP_H

#include <QObject>
#include <QTransform>
#include <QVector>
#include "photon-global.h"
#include "gizmohandle.h"

class QGraphicsScene;
class QGraphicsItem;
class QGraphicsPathItem;

namespace photon {

/**
 * Owns a set of data-space GizmoHandles plus the connector drawn between them,
 * and maps them through a data->screen transform supplied by the host viewer.
 *
 * An effect editor creates one group against its scene, adds handles with data
 * getters/setters, declares how they connect (straight lines or cubic beziers),
 * and calls setTransform() whenever the view pans/zooms. The group keeps the
 * handles positioned and the connector redrawn; the effect never touches pixels.
 *
 * Cubic connections take two control handles (typically Tangent handles anchored
 * relative to their endpoints), which is the primitive bezier spline editing is
 * built on.
 */
class PHOTONCORE_EXPORT GizmoGroup : public QObject
{
    Q_OBJECT
public:
    explicit GizmoGroup(QGraphicsScene *scene, QObject *parent = nullptr);
    ~GizmoGroup() override;

    GizmoHandle *addHandle(GizmoHandle::Style style = GizmoHandle::Anchor,
                           Qt::Orientations constraint = Qt::Horizontal | Qt::Vertical);

    // Connect two handles with a straight segment.
    void connectLine(GizmoHandle *from, GizmoHandle *to);
    // Connect two handles with a cubic bezier through the two control handles.
    void connectCubic(GizmoHandle *from, GizmoHandle *controlFrom,
                      GizmoHandle *controlTo, GizmoHandle *to);

    const QTransform &transform() const { return m_transform; }
    void setTransform(const QTransform &transform);   // pan/zoom: reposition + redraw
    void refresh();                                   // reposition handles + rebuild connector

    void setConnectorPen(const QPen &pen);
    void setVisible(bool visible);

    // Drag bookkeeping: the handle being dragged shouldn't be repositioned out
    // from under the cursor when a value change triggers a transform refresh.
    void setActiveHandle(GizmoHandle *handle) { m_active = handle; }
    GizmoHandle *activeHandle() const { return m_active; }
    void handleMoved(GizmoHandle *handle);            // a live drag moved a handle

private:
    struct Connection
    {
        GizmoHandle *from = nullptr;
        GizmoHandle *to = nullptr;
        GizmoHandle *controlFrom = nullptr;
        GizmoHandle *controlTo = nullptr;
    };

    void rebuildConnector();

    QGraphicsScene *m_scene;
    QGraphicsItem *m_root;
    QGraphicsPathItem *m_connector;
    QVector<GizmoHandle *> m_handles;
    QVector<Connection> m_connections;
    QTransform m_transform;
    GizmoHandle *m_active = nullptr;
};

} // namespace photon

#endif // PHOTON_GIZMOGROUP_H
