#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include "palettegizmoeditor.h"
#include "gui/color/colorpalettewidget.h"

namespace photon {

PaletteGizmoEditor::PaletteGizmoEditor(PaletteGizmo *t_gizmo, QWidget *parent)
    : QWidget{parent}
{
    QFormLayout *formLayout = new QFormLayout;

    QLineEdit *nameEdit = new QLineEdit;
    nameEdit->setText(t_gizmo->id());
    formLayout->addRow("Name", nameEdit);

    connect(nameEdit, &QLineEdit::textEdited, [t_gizmo](const QString &text){
        t_gizmo->setId(text.toLatin1());
    });
    /*
    QLineEdit *textEdit = new QLineEdit;
    textEdit->setText(t_gizmo->text());
    formLayout->addRow("Text", textEdit);

    connect(textEdit, &QLineEdit::textEdited, [t_gizmo](const QString &text){
        t_gizmo->setText(text);
    });
*/
    ColorPaletteWidget *paletteWidget = new ColorPaletteWidget(t_gizmo->palette(),true);

    paletteWidget->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));
    connect(paletteWidget, &ColorPaletteWidget::paletteUpdated, this,[t_gizmo, paletteWidget](){t_gizmo->setPalette(paletteWidget->palette());});
    formLayout->addRow("Colors", paletteWidget);

    QCheckBox *toggleCheck = new QCheckBox;
    toggleCheck->setChecked(t_gizmo->isSticky());
    formLayout->addRow("Sticky", toggleCheck);

    connect(toggleCheck, &QCheckBox::checkStateChanged, [t_gizmo](Qt::CheckState state){
        t_gizmo->setIsSticky(state == Qt::CheckState::Checked);
    });

    setLayout(formLayout);
}

} // namespace photon
