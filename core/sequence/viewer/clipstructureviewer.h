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
    QHash<QByteArray,QByteArray> m_states;

};

} // namespace photon

#endif // PHOTON_CLIPSTRUCTUREVIEWER_H
