#include <QHBoxLayout>
#include <QPushButton>
#include "togglegizmowidget.h"
#include "togglegizmoeditor.h"
#include "photoncore.h"
#include "gui/guimanager.h"

namespace photon {

ToggleGizmoWidget::ToggleGizmoWidget(ToggleGizmo *t_gizmo, SurfaceMode t_mode):SurfaceGizmoWidget(t_gizmo, t_mode),m_gizmo(t_gizmo)
{
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);
    m_button = new QPushButton(t_gizmo->text());



    m_button->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding));
    m_button->setMaximumHeight(1000);

    connect(m_button, &QAbstractButton::pressed, [t_gizmo, this](){
        t_gizmo->setIsPressed(true);
        t_gizmo->addHistoryEvent(ToggleGizmo::GizmoPressId);
        if(m_button->isChecked())
            t_gizmo->addHistoryEvent(ToggleGizmo::GizmoToggleOnId);
    });

    connect(m_button, &QAbstractButton::released, [t_gizmo, this](){
        if(t_gizmo->isSticky())

            t_gizmo->setIsPressed(m_button->isChecked());
        else
        {
            t_gizmo->addHistoryEvent(ToggleGizmo::GizmoToggleOffId);
            t_gizmo->setIsPressed(false);
        }

        t_gizmo->addHistoryEvent(ToggleGizmo::GizmoReleaseId);
    });



    hLayout->addWidget(m_button);

    if(t_mode == Mode_Edit)
    {
        QPushButton *editButton = new QPushButton("Edit");
        editButton->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::MinimumExpanding));
        editButton->setMaximumHeight(1000);
        hLayout->addWidget(editButton);

        connect(editButton, &QPushButton::clicked, [t_gizmo](){
            auto editWidget = new ToggleGizmoEditor(t_gizmo);
            photonApp->gui()->showPopoverWidget(editWidget);
        });
    }

    updateGizmo();

    setLayout(hLayout);
}

void ToggleGizmoWidget::updateGizmo()
{
    m_button->setStyleSheet("QPushButton { background-color: " + m_gizmo->offColor().name() + "; color: black; }"
                            "QPushButton:pressed, QPushButton:checked { background-color: " + m_gizmo->onColor().name() + "; color: black; }");

    m_button->setCheckable(m_gizmo->isSticky());
    m_button->setText(m_gizmo->text());

}

} // namespace photon
