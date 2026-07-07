#include <QQmlContext>
#include <QQmlEngine>
#include "surfacequickview.h"
#include "surface/surface.h"
#include "surface/surfacegizmo.h"
#include "surface/containergizmo.h"
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

    m_surface = t_surface;
    emit rootChanged();
}

Surface *SurfaceQuickView::surface() const
{
    return m_surface;
}

QObject *SurfaceQuickView::rootObject() const
{
    return m_surface ? static_cast<QObject*>(m_surface->rootContainer()) : nullptr;
}

void SurfaceQuickView::addGizmo(const QString &t_type, QObject *t_target)
{
    if(!m_surface)
        return;

    SurfaceGizmo *gizmo = GizmoFactory::createGizmo(t_type.toUtf8());
    if(!gizmo)
        return;

    ContainerGizmo *container = qobject_cast<ContainerGizmo*>(t_target);
    if(!container)
        container = m_surface->rootContainer();

    // Cascade the drop position within the target container (used by Absolute
    // layouts; layout containers arrange children themselves).
    const int n = container->children().size();
    gizmo->setPropertyValue("x", 20.0 + (n % 4) * 180.0);
    gizmo->setPropertyValue("y", 20.0 + (n / 4) * 90.0);

    container->addChild(gizmo);
}

void SurfaceQuickView::removeGizmo(QObject *t_gizmo)
{
    auto *gizmo = qobject_cast<SurfaceGizmo*>(t_gizmo);
    if(!gizmo || !m_surface)
        return;

    if(auto *container = m_surface->rootContainer()->parentContainerOf(gizmo))
        container->removeChild(gizmo);
}

} // namespace photon
