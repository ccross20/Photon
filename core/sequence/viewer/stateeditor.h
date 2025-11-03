#ifndef PHOTON_STATEEDITOR_H
#define PHOTON_STATEEDITOR_H

#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include "photon-global.h"

namespace photon {

class StateClip;

class StateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit StateEditor(QWidget *parent = nullptr);


signals:

public slots:
    void openAddMenu();
    void setBaseEffectParent(photon::BaseEffectParent* );
    void selectState(photon::State *);
    void capabilityAdded(photon::StateCapability *);
    void capabilityRemoved(photon::StateCapability *);
    void clearSelection();

private:
    BaseEffectParent *m_baseEffectParent = nullptr;
    State *m_state = nullptr;
    QPushButton *m_addButton = nullptr;
    QVBoxLayout *m_layout;

};

} // namespace photon

#endif // PHOTON_STATEEDITOR_H
