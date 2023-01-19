#include <QLabel>
#include "canvasviewerpanel.h"
#include "photoncore.h"
#include "project/project.h"
#include "pixel/canvascollection.h"
#include "pixel/canvas.h"

namespace photon {

class CanvasViewerPanel::Impl
{
public:
    QLabel *label;
};

CanvasViewerPanel::CanvasViewerPanel() : Panel("photon.canvas-viewer"),m_impl(new Impl)
{
    m_impl->label = new QLabel;
    setPanelWidget(m_impl->label);

    if(photonApp->project() && photonApp->project()->canvases()->canvasCount() > 0)
        connect(photonApp->project()->canvases()->canvases()[0], &Canvas::didPaint, this, &CanvasViewerPanel::canvasUpdated);
}

CanvasViewerPanel::~CanvasViewerPanel()
{
    delete m_impl;
}

void CanvasViewerPanel::canvasUpdated(const QImage &t_image)
{
    m_impl->label->setPixmap(QPixmap::fromImage(t_image));
}

} // namespace photon
