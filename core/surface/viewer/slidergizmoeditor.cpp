#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include "slidergizmoeditor.h"

namespace photon {

SliderGizmoEditor::SliderGizmoEditor(SliderGizmo *t_gizmo, QWidget *parent)
    : QWidget{parent}
{
    QFormLayout *formLayout = new QFormLayout;

    QLineEdit *nameEdit = new QLineEdit;
    nameEdit->setText(t_gizmo->id());
    formLayout->addRow("Name", nameEdit);

    connect(nameEdit, &QLineEdit::textEdited, [t_gizmo](const QString &text){
        t_gizmo->setId(text.toLatin1());
    });

    QLineEdit *textEdit = new QLineEdit;
    textEdit->setText(t_gizmo->text());
    formLayout->addRow("Text", textEdit);

    connect(textEdit, &QLineEdit::textEdited, [t_gizmo](const QString &text){
        t_gizmo->setText(text);
    });

    QDoubleSpinBox *minEdit = new QDoubleSpinBox;
    minEdit->setValue(t_gizmo->minValue());
    minEdit->setMaximum(100000);
    minEdit->setMinimum(-100000);
    formLayout->addRow("Min Value", minEdit);

    connect(minEdit, &QDoubleSpinBox::valueChanged, [t_gizmo](const double &value){
        t_gizmo->setMinValue(value);
    });

    QDoubleSpinBox *maxEdit = new QDoubleSpinBox;
    maxEdit->setValue(t_gizmo->maxValue());
    maxEdit->setMaximum(100000);
    maxEdit->setMinimum(-100000);
    formLayout->addRow("Max Value", maxEdit);

    connect(maxEdit, &QDoubleSpinBox::valueChanged, [t_gizmo](const double &value){
        t_gizmo->setMaxValue(value);
    });

    setLayout(formLayout);
}

} // namespace photon
