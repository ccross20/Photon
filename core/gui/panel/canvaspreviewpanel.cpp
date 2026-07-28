#include <QVBoxLayout>
#include <QComboBox>
#include <QTimer>
#include "canvaspreviewpanel.h"
#include "canvaspreviewwindow.h"
#include "graph/node/canvas/canvassubgraphnode.h"
#include "graph/node/canvas/canvasoutputnode.h"
#include "graph/node/canvas/canvasrendermanager.h"

namespace photon {

CanvasPreviewPanel::CanvasPreviewPanel() : Panel("photon.canvas-preview")
{
    setName("Canvas Preview");

    auto *root = new QWidget;
    auto *layout = new QVBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_combo = new QComboBox;
    layout->addWidget(m_combo);

    m_window = new CanvasPreviewWindow;
    layout->addWidget(QWidget::createWindowContainer(m_window, root), 1);

    connect(m_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CanvasPreviewPanel::selectionChanged);

    setPanelWidget(root);

    refreshList();

    // The set of canvases/outputs changes as the user edits graphs; keep it fresh.
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &CanvasPreviewPanel::refreshList);
    m_refreshTimer->start(1000);
}

CanvasPreviewPanel::~CanvasPreviewPanel()
{
}

void CanvasPreviewPanel::refreshList()
{
    auto *manager = CanvasRenderManager::instance();

    QVector<CanvasOutputNode *> outputs;
    QStringList labels;
    if (manager) {
        for (auto *canvas : manager->canvases()) {
            for (auto *output : canvas->outputNodes()) {
                outputs << output;
                labels << canvas->name() + " · " + output->name();
            }
        }
    }

    if (outputs == m_items)
        return;

    CanvasOutputNode *selected = (m_combo->currentIndex() >= 0 && m_combo->currentIndex() < m_items.size())
                                     ? m_items.at(m_combo->currentIndex()) : nullptr;

    m_items = outputs;
    QSignalBlocker block(m_combo);
    m_combo->clear();
    int reselect = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        m_combo->addItem(labels.at(i));
        if (m_items.at(i) == selected)
            reselect = i;
    }
    if (reselect < 0 && !m_items.isEmpty())
        reselect = 0;
    m_combo->setCurrentIndex(reselect);
    selectionChanged(reselect);
}

void CanvasPreviewPanel::selectionChanged(int index)
{
    CanvasOutputNode *node = (index >= 0 && index < m_items.size()) ? m_items.at(index) : nullptr;
    if (m_window)
        m_window->setOutputNode(node);
}

void CanvasPreviewPanel::previewOutput(CanvasOutputNode *output)
{
    refreshList();
    const int idx = m_items.indexOf(output);
    if (idx >= 0)
        m_combo->setCurrentIndex(idx);   // triggers selectionChanged -> setOutputNode
    else if (m_window)
        m_window->setOutputNode(output); // not listed yet; show it anyway
}

} // namespace photon
