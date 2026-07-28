#ifndef PHOTON_CANVASPREVIEWPANEL_H
#define PHOTON_CANVASPREVIEWPANEL_H

#include <QVector>
#include "photon-global.h"
#include "gui/panel.h"

class QComboBox;
class QTimer;

namespace photon {

class CanvasPreviewWindow;
class CanvasOutputNode;

// Panel that previews a canvas Output node's texture. A combo box picks which
// output (across all canvases) and an embedded QRhi window shows it live. The
// "View Preview" button on an Output node calls previewOutput() to open+focus it.
class PHOTONCORE_EXPORT CanvasPreviewPanel : public Panel
{
    Q_OBJECT
public:
    CanvasPreviewPanel();
    ~CanvasPreviewPanel();

    // Select and show the given Output node (opening/refreshing the list).
    void previewOutput(CanvasOutputNode *output);

private slots:
    void refreshList();
    void selectionChanged(int index);

private:
    CanvasPreviewWindow *m_window = nullptr;
    QComboBox *m_combo = nullptr;
    QTimer *m_refreshTimer = nullptr;
    QVector<CanvasOutputNode *> m_items;
};

} // namespace photon

#endif // PHOTON_CANVASPREVIEWPANEL_H
