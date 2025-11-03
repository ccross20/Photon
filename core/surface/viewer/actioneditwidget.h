#ifndef ACTIONEDITWIDGET_H
#define ACTIONEDITWIDGET_H

#include <QWidget>
#include "photon-global.h"


namespace photon {

class PHOTONCORE_EXPORT ActionEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ActionEditWidget(QWidget *parent = nullptr);
    ~ActionEditWidget();

    void setContainer(SurfaceGizmoContainer *);
    SurfaceGizmoContainer *container() const;

private slots:
    void selectionChanged();
    void selectEffect(photon::ChannelEffect *);
    void selectMask(photon::MaskEffect *);
    void selectActionParameter(photon::SurfaceAction *);
    void selectFalloff(photon::FalloffEffect *);
    void selectBaseEffect(photon::BaseEffect *);
    void selectState(photon::State *);
    void selectAction(photon::SurfaceAction *);
    void clearEditor();
    void addAction();
    void removeAction();

signals:
    void addedToSelection(photon::SurfaceAction*);
    void removedFromSelection(photon::SurfaceAction*);

protected:
    void showEvent(QShowEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // ACTIONEDITWIDGET_H
