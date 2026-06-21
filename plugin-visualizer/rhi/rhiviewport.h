#ifndef PHOTON_RHIVIEWPORT_H
#define PHOTON_RHIVIEWPORT_H

#include <QWidget>
#include "rhigizmo.h"

class QButtonGroup;
class QToolButton;

namespace photon {

class SceneObject;
class RhiWindow;
class DMXMatrix;

// Widget facade for the 3D viewport. Hosts a window-based QRhi renderer
// (RhiWindow) via QWidget::createWindowContainer so it can sit in the panel
// layout while keeping full control over the QRhi lifetime.
class RhiViewport : public QWidget
{
    Q_OBJECT
public:
    explicit RhiViewport(QWidget *parent = nullptr);

    void setSceneRoot(SceneObject *root);
    void setSelectedSceneObject(SceneObject *obj);
    void setDmxState(const DMXMatrix &dmx);

signals:
    void selectionChanged(photon::SceneObject *obj);

private slots:
    void onGizmoModeChanged(RhiGizmo::Mode mode);

private:
    RhiWindow    *m_window    = nullptr;
    QButtonGroup *m_modeGroup = nullptr;
    QToolButton  *m_spaceBtn  = nullptr;
};

} // namespace photon

#endif // PHOTON_RHIVIEWPORT_H
