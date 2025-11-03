#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include "actionsettingswidget.h"

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

    setLayout(formLayout);
}

} // namespace photon
