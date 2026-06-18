#include <QHBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include "slidergizmowidget.h"
#include "slidergizmoeditor.h"
#include "photoncore.h"
#include "gui/guimanager.h"

namespace photon {

CustomSlider::CustomSlider() : QSlider()
{

}
void CustomSlider::setText(const QString &t_text)
{
    m_text = t_text;
    update();
}

void CustomSlider::paintEvent(QPaintEvent *t_event)
{
    QPainter painter(this);


    double percent = value()/1000.0;

    QRect r = rect();
    r.setWidth(r.width() * percent);

    painter.fillRect(r, Qt::cyan);

    painter.drawText(rect(), Qt::AlignCenter | Qt::AlignVCenter, m_text);
}

SliderGizmoWidget::SliderGizmoWidget(SliderGizmo *t_gizmo, SurfaceMode t_mode):SurfaceGizmoWidget(t_gizmo, t_mode),m_gizmo(t_gizmo)
{
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);
    m_slider = new CustomSlider();
    m_slider->setMinimum(0);
    m_slider->setMaximum(1000);
    m_slider->setOrientation(Qt::Horizontal);
    m_slider->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding));
    m_slider->setMaximumHeight(1000);
    m_slider->setText(t_gizmo->text());
    hLayout->addWidget(m_slider);

    connect(m_slider, &QAbstractSlider::valueChanged, [t_gizmo](int value){
        double v = value/1000.0;
        t_gizmo->setValue(((t_gizmo->maxValue() - t_gizmo->minValue()) * v) + t_gizmo->minValue());

    });

    m_slider->setValue((t_gizmo->value() - t_gizmo->minValue())/(t_gizmo->maxValue() - t_gizmo->minValue())*1000.0);

    if(t_mode == Mode_Edit)
    {
        QPushButton *editButton = new QPushButton("Edit");
        editButton->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::MinimumExpanding));
        editButton->setMaximumHeight(1000);
        hLayout->addWidget(editButton);

        connect(editButton, &QPushButton::clicked, [t_gizmo](){
            auto editWidget = new SliderGizmoEditor(t_gizmo);
            photonApp->gui()->showPopoverWidget(editWidget);
        });
    }

   // updateGizmo();

    setLayout(hLayout);
}

void SliderGizmoWidget::updateGizmo()
{
    m_slider->setText(m_gizmo->text());
}

} // namespace photon
