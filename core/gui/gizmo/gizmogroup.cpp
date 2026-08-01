#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QPen>
#include "gizmogroup.h"

namespace photon {

GizmoGroup::GizmoGroup(QGraphicsScene *t_scene, QObject *t_parent)
    : QObject(t_parent), m_scene(t_scene)
{
    // A root item parents the connector and handles so tearing down the group
    // removes everything from the scene in one go. It stays at the origin with no
    // transform, so child positions are plain screen coordinates.
    m_root = new QGraphicsRectItem();
    m_scene->addItem(m_root);

    m_connector = new QGraphicsPathItem(m_root);
    m_connector->setPen(QPen(Qt::cyan, 2));
    m_connector->setBrush(Qt::NoBrush);
    m_connector->setZValue(0);
}

GizmoGroup::~GizmoGroup()
{
    // Deleting the root deletes the connector and all handles (its children).
    if(m_scene)
        m_scene->removeItem(m_root);
    delete m_root;
}

GizmoHandle *GizmoGroup::addHandle(GizmoHandle::Style t_style, Qt::Orientations t_constraint)
{
    auto *handle = new GizmoHandle(this, t_style);
    handle->setConstraint(t_constraint);
    handle->setParentItem(m_root);
    handle->setZValue(1);
    m_handles.append(handle);
    return handle;
}

void GizmoGroup::connectLine(GizmoHandle *t_from, GizmoHandle *t_to)
{
    m_connections.append(Connection{t_from, t_to, nullptr, nullptr});
    rebuildConnector();
}

void GizmoGroup::connectCubic(GizmoHandle *t_from, GizmoHandle *t_controlFrom,
                             GizmoHandle *t_controlTo, GizmoHandle *t_to)
{
    m_connections.append(Connection{t_from, t_to, t_controlFrom, t_controlTo});
    rebuildConnector();
}

void GizmoGroup::setTransform(const QTransform &t_transform)
{
    m_transform = t_transform;
    refresh();
}

void GizmoGroup::refresh()
{
    for(GizmoHandle *handle : m_handles)
    {
        if(handle != m_active)
            handle->refreshFromData();
    }
    rebuildConnector();
}

void GizmoGroup::setConnectorPen(const QPen &t_pen)
{
    m_connector->setPen(t_pen);
}

void GizmoGroup::setVisible(bool t_visible)
{
    m_root->setVisible(t_visible);
}

void GizmoGroup::handleMoved(GizmoHandle *)
{
    // Live drag: the moved handle's scene pos is already current, so just redraw
    // the connector to follow it. Other handles may depend on it (e.g. tangents
    // relative to an anchor), so refresh their positions too.
    for(GizmoHandle *handle : m_handles)
    {
        if(handle != m_active)
            handle->refreshFromData();
    }
    rebuildConnector();
}

void GizmoGroup::rebuildConnector()
{
    QPainterPath path;
    for(const Connection &c : m_connections)
    {
        if(!c.from || !c.to)
            continue;

        path.moveTo(c.from->pos());
        if(c.controlFrom && c.controlTo)
            path.cubicTo(c.controlFrom->pos(), c.controlTo->pos(), c.to->pos());
        else
            path.lineTo(c.to->pos());
    }
    m_connector->setPath(path);
}

} // namespace photon
