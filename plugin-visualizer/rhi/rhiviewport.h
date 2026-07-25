#ifndef PHOTON_RHIVIEWPORT_H
#define PHOTON_RHIVIEWPORT_H

#include <QWidget>
#include <QVector>
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
    void setSelectedSceneObjects(const QVector<SceneObject *> &objs);
    void setDmxState(const DMXMatrix &dmx);

signals:
    void selectionChanged(const QVector<photon::SceneObject *> &objs);

private slots:
    void onGizmoModeChanged(RhiGizmo::Mode mode);

private:
    RhiWindow    *m_window    = nullptr;
    QButtonGroup *m_modeGroup = nullptr;
    QToolButton  *m_spaceBtn  = nullptr;
    QToolButton  *m_beamBtn   = nullptr;
    QToolButton  *m_goboBtn   = nullptr;
    int           m_goboIndex = 0;
};

} // namespace photon

#endif // PHOTON_RHIVIEWPORT_H
