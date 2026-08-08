#ifndef PHOTON_CANVASPREVIEWPANEL_H
#define PHOTON_CANVASPREVIEWPANEL_H

#include <QVector>
#include "photon-global.h"
#include "gui/panel.h"
#include "canvaspreviewwindow.h"

class QComboBox;
class QTimer;

namespace photon {

class CanvasOutputNode;
class CanvasLayerGroup;

// Panel that previews a canvas Output node's or CanvasLayerGroup's texture. A
// combo box picks which target (across all canvases and layer groups) and an
// embedded QRhi window shows it live. The "View Preview" button on an Output
// node or a CanvasLayerGroup's editor calls previewOutput()/previewLayerGroup()
// to open+focus it.
class PHOTONCORE_EXPORT CanvasPreviewPanel : public Panel
{
    Q_OBJECT
public:
    CanvasPreviewPanel();
    ~CanvasPreviewPanel();

    // Select and show the given Output node (opening/refreshing the list).
    void previewOutput(CanvasOutputNode *output);
    // Select and show the given CanvasLayerGroup's own sink.
    void previewLayerGroup(CanvasLayerGroup *group);

private slots:
    void refreshList();
    void selectionChanged(int index);

private:
    CanvasPreviewWindow *m_window = nullptr;
    QComboBox *m_combo = nullptr;
    QTimer *m_refreshTimer = nullptr;
    QVector<CanvasPreviewTarget> m_items;
};

} // namespace photon

#endif // PHOTON_CANVASPREVIEWPANEL_H
