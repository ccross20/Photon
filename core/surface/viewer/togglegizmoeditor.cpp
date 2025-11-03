#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include "togglegizmoeditor.h"
#include "gui/color/colorwheelswatch.h"

namespace photon {

ToggleGizmoEditor::ToggleGizmoEditor(ToggleGizmo *t_gizmo, QWidget *parent)
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

    ColorWheelSwatch *offSwatch = new ColorWheelSwatch();
    offSwatch->setColor(t_gizmo->offColor());
    offSwatch->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));
    ColorWheelSwatch::connect(offSwatch, &ColorWheelSwatch::colorChanged, offSwatch,[t_gizmo](QColor color){t_gizmo->setOffColor(color);});
    formLayout->addRow("Off Color", offSwatch);

    ColorWheelSwatch *onSwatch = new ColorWheelSwatch();
    onSwatch->setColor(t_gizmo->onColor());
    onSwatch->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));
    ColorWheelSwatch::connect(onSwatch, &ColorWheelSwatch::colorChanged, onSwatch,[t_gizmo](QColor color){t_gizmo->setOnColor(color);});
    formLayout->addRow("On Color", onSwatch);

    QCheckBox *toggleCheck = new QCheckBox;
    toggleCheck->setChecked(t_gizmo->isSticky());
    formLayout->addRow("Sticky", toggleCheck);

    connect(toggleCheck, &QCheckBox::checkStateChanged, [t_gizmo](Qt::CheckState state){
        t_gizmo->setIsSticky(state == Qt::CheckState::Checked);
    });

    setLayout(formLayout);
}

} // namespace photon
