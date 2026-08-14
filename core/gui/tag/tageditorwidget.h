#ifndef PHOTON_TAGEDITORWIDGET_H
#define PHOTON_TAGEDITORWIDGET_H

#include <QWidget>
#include <functional>
#include "photon-global.h"

class QLineEdit;
class QCompleter;

namespace photon {

class FlowLayout;

// The reusable tag picker/editor: a wrapping row of colored TagChips plus a
// trailing "add" field that autocompletes from a caller-supplied pool of
// known tags, or creates a new one. Driven entirely through get/set/knownTags
// callbacks rather than a fixed data type, so the same widget edits a
// resource's own tags, a FixtureQuery's match-predicate tags, or a graph
// node's tags - different lists, same look and the same picker.
class PHOTONCORE_EXPORT TagEditorWidget : public QWidget
{
    Q_OBJECT
public:
    TagEditorWidget(std::function<QStringList()> get,
                    std::function<void(QStringList)> set,
                    std::function<QStringList()> knownTags,
                    QWidget *parent = nullptr);
    ~TagEditorWidget() override;

public slots:
    // Rebuilds the chip row from get(). Call when the underlying list may
    // have changed from outside this widget.
    void refresh();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void chipRemoveRequested(QString tag);
    void commitPendingText();
    void completionActivated(const QString &text);

private:
    void addTags(const QStringList &tags);
    void rebuildCompleter();

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TAGEDITORWIDGET_H
