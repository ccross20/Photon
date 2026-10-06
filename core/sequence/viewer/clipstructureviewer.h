#ifndef PHOTON_CLIPSTRUCTUREVIEWER_H
#define PHOTON_CLIPSTRUCTUREVIEWER_H

#include <QWidget>
#include <QTreeView>
#include <QPersistentModelIndex>
#include "sequence/viewer/clipmodel.h"

namespace photon {

class ClipTreeView : public QTreeView
{
    Q_OBJECT
public:
    ClipTreeView();

signals:
    void effectCreated(photon::ChannelEffect *);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    // Runs after the click that asked for it has been fully handled, so no
    // part of that click's own selection handling can override selecting the
    // new effect.
    void showAddEffectMenu(photon::Channel *channel, const QPoint &globalPos);
};



class ClipStructureViewer : public QWidget
{
    Q_OBJECT
public:
    explicit ClipStructureViewer(QWidget *parent = nullptr);
    ~ClipStructureViewer();

    void setClip(Clip *);
    void restoreState();
    void viewId(const QByteArray &);

signals:
    void selectEffect(photon::ChannelEffect *);
    void selectClipGraph(photon::Clip*);
    // The clip's own top-level row was selected: show the clip's properties.
    void selectClipProperties(photon::Clip*);
    void selectPixelLayout(photon::PixelLayout *);
    void clearSelection();

private slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void selectEffectRow(photon::ChannelEffect *);
    void rowsRemoved();

private:
    ClipTreeView *m_treeView;
    ClipModel *m_model;
    Clip *m_clip = nullptr;
    // Whether an effect or clip graph is selected (and so open in the editor).
    bool m_hasEditorSelection = false;
    // Set while a clip is swapped in or its view restored.
    bool m_switching = false;
    // Per clip (by uniqueId): the id of the row last selected for it (the
    // clip itself, its graph, or an effect), so selecting the clip again
    // reopens the same view.
    QHash<QByteArray,QByteArray> m_states;

};

} // namespace photon

#endif // PHOTON_CLIPSTRUCTUREVIEWER_H
