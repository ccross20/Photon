#ifndef PHOTON_RESOURCEEDITORWIDGET_H
#define PHOTON_RESOURCEEDITORWIDGET_H

#include <QWidget>
#include <functional>
#include "photon-global.h"

namespace photon {

// Name + tags editor for any ProjectResource. Used on its own as the default
// Properties-panel editor, and embedded at the top of the richer per-type
// editors so every resource edits its shared metadata the same way.
class PHOTONCORE_EXPORT ResourceEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ResourceEditorWidget(ProjectResource *resource, QWidget *parent = nullptr);
    ~ResourceEditorWidget();

    // Adds a button that opens the resource's full editor - the routine graph,
    // the sequence timeline, the surface designer. Those stay as their own
    // dockable panels; this is just the way in from the Properties panel.
    void setOpenAction(const QString &label, std::function<void()> callback);

private slots:
    void nameEdited();
    // The resource changed underneath us (renamed elsewhere, retagged by the
    // project panel) - refresh the fields without echoing back.
    void resourceChanged();
    // The resource was destroyed while its editor was still on screen. Drop the
    // pointer and go read-only rather than dangling.
    void resourceDestroyed();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_RESOURCEEDITORWIDGET_H
