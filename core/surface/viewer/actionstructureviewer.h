#ifndef ACTIONSTRUCTUREVIEWER_H
#define ACTIONSTRUCTUREVIEWER_H


#include <QWidget>
#include <QTreeView>
#include "actionmodel.h"

namespace photon {

class SurfaceGizmoContainer;

class ActionTreeView : public QTreeView
{
    Q_OBJECT
public:
    ActionTreeView();

protected:
    void mousePressEvent(QMouseEvent *event) override;
};



class ActionStructureViewer : public QWidget
{
    Q_OBJECT
public:
    explicit ActionStructureViewer(QWidget *parent = nullptr);
    ~ActionStructureViewer();

    void setContainer(SurfaceGizmoContainer *);
    void restoreState();
    void viewId(const QByteArray &);

signals:
    void selectEffect(photon::ChannelEffect *);
    void selectFalloff(photon::FalloffEffect *);
    void selectMask(photon::MaskEffect *);
    void selectState(photon::State *);
    void selectBaseEffect(photon::BaseEffect *);
    void selectAction(photon::SurfaceAction *);
    void selectActionParameter(photon::SurfaceAction*);
    void clearSelection();

private slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    ActionTreeView *m_treeView;
    SurfaceGizmoContainer *m_container = nullptr;
    ActionModel *m_model;
    QHash<QByteArray,QByteArray> m_states;

};

} // namespace photon

#endif // ACTIONSTRUCTUREVIEWER_H
