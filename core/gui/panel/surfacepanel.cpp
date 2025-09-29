#include <QKeyEvent>
#include "surfacepanel.h"
#include "surface/surface.h"
#include "photoncore.h"
#include "surface/viewer/surfacewidget.h"

namespace photon {


class SurfacePanel::Impl
{
public:
    SurfaceWidget *surfaceViewer;
};

SurfacePanel::SurfacePanel() : Panel("photon.surface"),m_impl(new Impl)
{

    setName("Surface");
    m_impl->surfaceViewer = new SurfaceWidget;

    setPanelWidget(m_impl->surfaceViewer);

}

SurfacePanel::~SurfacePanel()
{
    //if(photonApp->sequences()->activeSequencePanel() == this)
        //photonApp->sequences()->setActiveSequencePanel(nullptr);
    delete m_impl;
}

void SurfacePanel::processPreview(ProcessContext &context)
{


}


void SurfacePanel::setSurface(Surface *t_surface)
{
    m_impl->surfaceViewer->setSurface(t_surface);
}

Surface *SurfacePanel::surface() const
{
    return m_impl->surfaceViewer->surface();
}

bool SurfacePanel::panelKeyPress(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Space)
    {
        //m_impl->surfaceViewer->togglePlay(!m_impl->surfaceViewer->isPlaying());
        return true;
    }
    return false;
}

bool SurfacePanel::panelKeyRelease(QKeyEvent *event)
{
    return false;
}

void SurfacePanel::didActivate()
{
    Panel::didActivate();
}

void SurfacePanel::didDeactivate()
{
    Panel::didDeactivate();


}

} // namespace photon
