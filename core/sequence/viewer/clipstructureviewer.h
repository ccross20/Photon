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

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    // Selecting the newly-created effect's row right after channel->addEffect()
    // doesn't stick - something later in the same gesture (the "Add Effect..."
    // click opens a modal QMenu mid-mousePressEvent) re-applies the previous
    // selection afterward. Re-assert once more on the next event loop turn so
    // the effect wins regardless of what raced it.
    void reassertSelection(const QPersistentModelIndex &index);
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

private:
    ClipTreeView *m_treeView;
    ClipModel *m_model;
    Clip *m_clip = nullptr;
    QHash<QByteArray,QByteArray> m_states;

};

} // namespace photon

#endif // PHOTON_CLIPSTRUCTUREVIEWER_H
