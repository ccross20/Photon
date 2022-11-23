#include <QVBoxLayout>
#include <QFormLayout>
#include <QSlider>
#include "parametereditor.h"

ParameterEditor::ParameterEditor(QWidget *parent)
    : QWidget{parent}
{
    QFormLayout *form = new QFormLayout;

    QSlider *beamSlider = new QSlider(Qt::Horizontal);
    beamSlider->setRange(3,6);
    beamSlider->setValue(4);
    connect(beamSlider, &QSlider::valueChanged,this, [this](int val){ emit beamChanged(val);});
    form->addRow("Beams", beamSlider);

    QSlider *segmentSlider = new QSlider(Qt::Horizontal);
    segmentSlider->setValue(20);
    segmentSlider->setRange(1,100);
    connect(segmentSlider, &QSlider::valueChanged,this, [this](int val){ emit segmentChanged(val/10.0);});
    form->addRow("Segments", segmentSlider);


    QSlider *radiusSlider = new QSlider(Qt::Horizontal);
    radiusSlider->setRange(2,20);
    radiusSlider->setValue(30);
    connect(radiusSlider, &QSlider::valueChanged,this, [this](int val){ emit radiusChanged(val/10.0);});
    form->addRow("Radius", radiusSlider);


    QSlider *offsetSlider = new QSlider(Qt::Horizontal);
    offsetSlider->setRange(1,100);
    offsetSlider->setValue(20);
    connect(offsetSlider, &QSlider::valueChanged,this, [this](int val){ emit offsetChanged(val/10.0);});
    form->addRow("Offset", offsetSlider);


    QSlider *lengthSlider = new QSlider(Qt::Horizontal);
    lengthSlider->setRange(1,200);
    lengthSlider->setValue(50);
    connect(lengthSlider, &QSlider::valueChanged,this, [this](int val){ emit lengthChanged(val/2.5);});
    form->addRow("Length", lengthSlider);


    QSlider *angleSlider = new QSlider(Qt::Horizontal);
    angleSlider->setValue(0);
    angleSlider->setRange(-360,360);
    connect(angleSlider, &QSlider::valueChanged,this, [this](int val){ emit angleChanged(val);});
    form->addRow("Angle", angleSlider);

    setLayout(form);
}
