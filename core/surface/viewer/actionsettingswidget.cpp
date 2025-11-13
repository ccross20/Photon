#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include "actionsettingswidget.h"
#include "surface/surfacegizmocontainer.h"
#include "surface/togglegizmo.h"

namespace photon {

ActionSettingsWidget::ActionSettingsWidget(SurfaceAction *t_action, QWidget *parent)
    : QWidget{parent}
{

    QFormLayout *formLayout = new QFormLayout;

    QDoubleSpinBox *durationEdit = new QDoubleSpinBox();
    durationEdit->setRange(0.0,100.0);
    durationEdit->setValue(t_action->duration());
    formLayout->addRow("Duration", durationEdit);

    connect(durationEdit, &QDoubleSpinBox::valueChanged, [t_action](double value){
        t_action->setDuration(value);
    });


    QComboBox *enableCombo = new QComboBox;
    enableCombo->addItem("Always","");

    auto gizmos = t_action->parentContainer()->gizmosByType(ToggleGizmo::GizmoId);
    int counter = 1;
    int matchIndex = -1;
    for(auto gizmo : gizmos){
        enableCombo->addItem(gizmo->id(),gizmo->uniqueId());

        if(t_action->enableOnGizmoId() == gizmo->uniqueId())
            matchIndex = counter;
        counter++;
    }

    if(matchIndex >= 0)
        enableCombo->setCurrentIndex(matchIndex);

    connect(enableCombo, &QComboBox::currentIndexChanged, this, [enableCombo, t_action](int index){
        t_action->setEnableOnGizmoId(enableCombo->currentData().toByteArray());
    });

    formLayout->addRow("Enable On:", enableCombo);

    setLayout(formLayout);
}

} // namespace photon
