#ifndef PHOTON_CANVASPREVIEWPANEL_H
#define PHOTON_CANVASPREVIEWPANEL_H

#include <QVector>
#include "photon-global.h"
#include "gui/panel.h"

class QComboBox;
class QTimer;

namespace photon {

class CanvasPreviewWindow;
class CanvasSubGraphNode;

// Panel that previews a canvas subgraph's rendered output. A combo box picks which
// canvas (populated from the CanvasRenderManager) and an embedded QRhi window shows
// it live.
class PHOTONCORE_EXPORT CanvasPreviewPanel : public Panel
{
    Q_OBJECT
public:
    CanvasPreviewPanel();
    ~CanvasPreviewPanel();

private slots:
    void refreshList();
    void selectionChanged(int index);

private:
    CanvasPreviewWindow *m_window = nullptr;
    QComboBox *m_combo = nullptr;
    QTimer *m_refreshTimer = nullptr;
    QVector<CanvasSubGraphNode *> m_items;
};

} // namespace photon

#endif // PHOTON_CANVASPREVIEWPANEL_H
