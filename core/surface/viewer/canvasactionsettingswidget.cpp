#include <QFormLayout>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QMenu>
#include <QDoubleSpinBox>
#include "canvasactionsettingswidget.h"
#include "pixel/pixellayoutcollection.h"
#include "pixel/canvascollection.h"
#include "photoncore.h"
#include "project/project.h"
#include "pixel/canvas.h"
#include "pixel/pixellayout.h"

namespace photon {

CanvasActionSettingsWidget::CanvasActionSettingsWidget(CanvasAction *t_action, QWidget *parent)
    : QWidget{parent},m_canvasAction(t_action)
{
    QFormLayout *formLayout = new QFormLayout;

    QDoubleSpinBox *durationEdit = new QDoubleSpinBox();
    durationEdit->setRange(0.0,100.0);
    durationEdit->setValue(t_action->duration());
    formLayout->addRow("Duration", durationEdit);

    connect(durationEdit, &QDoubleSpinBox::valueChanged, [t_action](double value){
        t_action->setDuration(value);
    });




    int currentIndex = 0;
    int counter = 0;
    m_canvasCombo = new QComboBox;
    for(auto canvas : photonApp->project()->canvases()->canvases())
    {
        m_canvasCombo->addItem(canvas->name());
        if(m_canvasAction->canvas() == canvas)
            currentIndex = counter;
        counter++;
    }
    m_canvasCombo->setCurrentIndex(currentIndex);

    formLayout->addRow("Canvas", m_canvasCombo);


    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0,0,0,0);

    m_pixelLayoutList = new QListWidget;

    for(auto layout : m_canvasAction->pixelLayouts())
    {
        m_pixelLayoutList->addItem(layout->name());
    }


    vLayout->addWidget(m_pixelLayoutList);

    m_addLayoutButton = new QPushButton("Add");
    connect(m_addLayoutButton, &QPushButton::clicked,this, &CanvasActionSettingsWidget::openAddPixelLayout);

    m_removeLayoutButton = new QPushButton("Remove");
    connect(m_removeLayoutButton, &QPushButton::clicked,this, &CanvasActionSettingsWidget::removeSelectedLayout);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->addWidget(m_addLayoutButton);
    hLayout->addWidget(m_removeLayoutButton);

    vLayout->addLayout(hLayout);
    formLayout->addRow("Pixel Layouts", vLayout);

    setLayout(formLayout);

}


void CanvasActionSettingsWidget::canvasChanged(int t_index)
{
    m_canvasAction->setCanvas(photonApp->project()->canvases()->canvases()[t_index]);
}

void CanvasActionSettingsWidget::openAddPixelLayout()
{
    QMenu menu;
    auto pixelLayouts = photonApp->project()->pixelLayouts()->layouts();
    for(auto layout : pixelLayouts)
    {
        if(!m_canvasAction->pixelLayouts().contains(layout))
            menu.addAction(layout->name(),[this, layout](){m_canvasAction->addPixelLayout(layout);});
    }
    menu.exec(m_addLayoutButton->mapToGlobal(QPoint(0,m_addLayoutButton->height())));
}

void CanvasActionSettingsWidget::removeSelectedLayout()
{
    QByteArrayList ids;
    for(auto item : m_pixelLayoutList->selectedItems())
    {
        ids << item->data(Qt::ItemDataRole::UserRole).toByteArray();
    }

    for(const auto &id : ids)
    {
        for(auto layout : m_canvasAction->pixelLayouts())
        {
            if(layout->uniqueId() == id)
                m_canvasAction->removePixelLayout(layout);
        }
    }

}

void CanvasActionSettingsWidget::pixelLayoutAdded(photon::PixelLayout *t_layout)
{
    auto item = new QListWidgetItem(t_layout->name());
    item->setData(Qt::ItemDataRole::UserRole, t_layout->uniqueId());
    m_pixelLayoutList->addItem(item);
}

void CanvasActionSettingsWidget::pixelLayoutRemoved(photon::PixelLayout *t_layout)
{
    for(int i = 0; i < m_pixelLayoutList->count(); ++i)
    {
        auto item = m_pixelLayoutList->item(i);
        if(item->data(Qt::ItemDataRole::UserRole).toByteArray() == t_layout->uniqueId())
        {
            delete item;
        }
    }
}


} // namespace photon
