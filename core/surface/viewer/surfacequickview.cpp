#include <QQmlContext>
#include <QQmlEngine>
#include "surfacequickview.h"
#include "surface/surface.h"
#include "surface/surfacegizmo.h"
#include "surface/surfacegizmocontainer.h"
#include "surface/gizmofactory.h"

namespace photon {

SurfaceQuickView::SurfaceQuickView(bool t_editMode, QWidget *parent)
    : QQuickWidget(parent), m_editMode(t_editMode)
{
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    rootContext()->setContextProperty("surfaceView", this);
    setSource(QUrl(m_editMode ? "qrc:/qml/surface/SurfaceEdit.qml"
                              : "qrc:/qml/surface/SurfacePerform.qml"));
}

bool SurfaceQuickView::editMode() const
{
    return m_editMode;
}

void SurfaceQuickView::setSurface(Surface *t_surface)
{
    if(m_surface == t_surface)
        return;

    if(m_surface)
        disconnect(m_surface, nullptr, this, nullptr);

    m_surface = t_surface;

    if(m_surface)
        connect(m_surface, &Surface::surfaceWasModified, this, &SurfaceQuickView::gizmosChanged);

    emit gizmosChanged();
}

Surface *SurfaceQuickView::surface() const
{
    return m_surface;
}

QVariantList SurfaceQuickView::gizmos() const
{
    QVariantList list;
    if(m_surface)
    {
        for(auto *gizmo : m_surface->gizmos())
            list.append(QVariant::fromValue(static_cast<QObject*>(gizmo)));
    }
    return list;
}

void SurfaceQuickView::addGizmo(const QString &t_type)
{
    if(!m_surface)
        return;

    SurfaceGizmo *gizmo = GizmoFactory::createGizmo(t_type.toUtf8());
    if(!gizmo)
        return;

    // Cascade the drop position across the whole surface so new gizmos don't
    // land on top of each other.
    const int n = m_surface->gizmos().size();
    gizmo->setPropertyValue("x", 20.0 + (n % 4) * 180.0);
    gizmo->setPropertyValue("y", 20.0 + (n / 4) * 90.0);

    // Ensure there is a container to hold it.
    SurfaceGizmoContainer *container = m_surface->containers().isEmpty()
        ? nullptr : m_surface->containers().first();
    if(!container)
    {
        container = new SurfaceGizmoContainer;
        m_surface->addGizmoContainer(container);
    }
    container->addGizmo(gizmo);

    emit gizmosChanged();
}

void SurfaceQuickView::removeGizmo(QObject *t_gizmo)
{
    auto *gizmo = qobject_cast<SurfaceGizmo*>(t_gizmo);
    if(!gizmo || !m_surface)
        return;

    for(auto *container : m_surface->containers())
    {
        if(container->gizmos().contains(gizmo))
        {
            container->removeGizmo(gizmo);
            break;
        }
    }

    emit gizmosChanged();
}

} // namespace photon
