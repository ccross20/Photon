#ifndef PHOTON_CLIPSTRUCTUREVIEWER_H
#define PHOTON_CLIPSTRUCTUREVIEWER_H

#include <QWidget>
#include <QTreeView>
#include "sequence/viewer/clipmodel.h"

namespace photon {

class ClipTreeView : public QTreeView
{
    Q_OBJECT
public:
    ClipTreeView();

protected:
    void mousePressEvent(QMouseEvent *event) override;
};



class ClipStructureViewer : public QWidget
{
    Q_OBJECT
public:
    explicit ClipStructureViewer(QWidget *parent = nullptr);
    ~ClipStructureViewer();

    void setClip(Clip *);
    void addMasterLayer(MasterLayer *);
    void removeMasterLayer(MasterLayer *);
    void restoreState();
    void viewId(const QByteArray &);

signals:
    void selectEffect(photon::ChannelEffect *);
    void selectFalloff(photon::FalloffEffect *);
    void selectMask(photon::MaskEffect *);
    void selectState(photon::State *);
    void selectClipEffect(photon::ClipEffect *);
    void selectClipParameter(photon::Clip*);
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
