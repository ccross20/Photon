#include <QQmlContext>
#include <QQmlEngine>
#include "surfacequickview.h"
#include "surface/surface.h"
#include "surface/surfacegizmo.h"

namespace photon {

SurfaceQuickView::SurfaceQuickView(QWidget *parent)
    : QQuickWidget(parent)
{
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    rootContext()->setContextProperty("surfaceView", this);
    setSource(QUrl("qrc:/qml/surface/SurfacePerform.qml"));
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

} // namespace photon
