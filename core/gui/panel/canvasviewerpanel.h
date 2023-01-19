#ifndef PHOTON_CANVASVIEWERPANEL_H
#define PHOTON_CANVASVIEWERPANEL_H

#include "photon-global.h"
#include "gui/panel.h"


namespace photon {

class PHOTONCORE_EXPORT CanvasViewerPanel : public Panel
{
    Q_OBJECT
public:
    CanvasViewerPanel();
    ~CanvasViewerPanel();

private slots:
    void canvasUpdated(const QImage &);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CANVASVIEWERPANEL_H
