#ifndef SURFACEPANEL_H
#define SURFACEPANEL_H
#include "photon-global.h"
#include "gui/panel.h"

namespace photon {

class PHOTONCORE_EXPORT SurfacePanel : public Panel
{
    Q_OBJECT
public:
    SurfacePanel();
    ~SurfacePanel();
    void setSurface(Surface *);
    Surface *surface() const;

    void processPreview(ProcessContext &context);


protected:

    void didDeactivate() override;
    void didActivate() override;
    bool panelKeyPress(QKeyEvent *event) override;
    bool panelKeyRelease(QKeyEvent *event) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // SURFACEPANEL_H
