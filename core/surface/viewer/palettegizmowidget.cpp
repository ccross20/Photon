#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include "palettegizmowidget.h"
#include "photoncore.h"
#include "gui/guimanager.h"
#include "palettegizmoeditor.h"
#include "util/utils.h"

namespace photon {

PaletteGizmoWidget::PaletteGizmoWidget(PaletteGizmo *t_gizmo, SurfaceMode t_mode):SurfaceGizmoWidget(t_gizmo, t_mode),m_gizmo(t_gizmo)
{
    m_layout = new QHBoxLayout;
    m_layout->setContentsMargins(0,0,0,0);
    m_mode = t_mode;
    m_group = new QButtonGroup;
    m_group->setExclusive(false);


    setLayout(m_layout);

    updateGizmo();
}

void PaletteGizmoWidget::updateGizmo()
{
    auto buttons = m_group->buttons();
    for(auto button : buttons)
        m_group->removeButton(button);


    clearLayout(m_layout);

    int counter = 0;
    for(const QColor color : m_gizmo->palette())
    {
        QPushButton *colorButton = new QPushButton();
        colorButton->setProperty("index", counter++);
        colorButton->setStyleSheet("QPushButton { background-color: " + color.name() + ";}"
                                   "QPushButton::pressed, QPushButton::checked{border-width:5px;border-color:white;border-style: outset;}");
        colorButton->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding));
        colorButton->setMaximumHeight(1000);
        colorButton->setCheckable(m_gizmo->isSticky());
        colorButton->setChecked(m_gizmo->selectedColor() == counter-1);
        m_group->addButton(colorButton);

        m_layout->addWidget(colorButton);


        connect(colorButton, &QAbstractButton::pressed, [this, counter](){
            m_gizmo->selectColor(counter-1);
            m_gizmo->setIsPressed(true);
        });


        connect(colorButton, &QAbstractButton::released, [this, colorButton](){

            if(m_gizmo->isSticky())
                m_gizmo->setIsPressed(colorButton->isChecked());
            else
            {
                m_gizmo->setIsPressed(false);
            }
        });


        connect(m_gizmo, &SurfaceGizmo::gizmoUpdated, colorButton, [this, colorButton](){
            colorButton->setChecked(m_gizmo->selectedColorIndex() == colorButton->property("index").toInt());
        });

    }

    if(m_mode == Mode_Edit)
    {
        QPushButton *editButton = new QPushButton("Edit");
        editButton->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::MinimumExpanding));
        editButton->setMaximumHeight(1000);
        m_layout->addWidget(editButton);


        connect(editButton, &QPushButton::clicked, [this](){
            auto editWidget = new PaletteGizmoEditor(m_gizmo);
            photonApp->gui()->showPopoverWidget(editWidget);
        });

    }
}

} // namespace photon
