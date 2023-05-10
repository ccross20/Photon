#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "truss.h"
#include "gui/vector3edit.h"

namespace photon {



class TrussEditorWidget::Impl
{
public:
    Impl();
    Truss *truss;
    QFormLayout *formLayout;
    QLineEdit *nameEdit;
    QSpinBox *beamSpin;
    QDoubleSpinBox *segmentLengthSpin;
    QDoubleSpinBox *radiusSpin;
    QDoubleSpinBox *offsetSpin;
    QDoubleSpinBox *lengthSpin;
    QDoubleSpinBox *angleSpin;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
};

TrussEditorWidget::Impl::Impl()
{
    formLayout = new QFormLayout;

    nameEdit = new QLineEdit;
    formLayout->addRow("Name", nameEdit);



    beamSpin = new QSpinBox;
    beamSpin->setMinimum(2);
    beamSpin->setMaximum(12);
    formLayout->addRow("Beams", beamSpin);

    segmentLengthSpin = new QDoubleSpinBox;
    segmentLengthSpin->setMinimum(.1);
    segmentLengthSpin->setMaximum(1);
    formLayout->addRow("Segment Length", segmentLengthSpin);

    radiusSpin = new QDoubleSpinBox;
    radiusSpin->setMinimum(.005);
    radiusSpin->setMaximum(.1);
    formLayout->addRow("Radius", radiusSpin);

    offsetSpin = new QDoubleSpinBox;
    offsetSpin->setMinimum(.005);
    offsetSpin->setMaximum(.2);
    formLayout->addRow("Offset", offsetSpin);

    lengthSpin = new QDoubleSpinBox;
    lengthSpin->setMinimum(.1);
    lengthSpin->setMaximum(20);
    formLayout->addRow("Length", lengthSpin);

    angleSpin = new QDoubleSpinBox;
    angleSpin->setMinimum(-180);
    angleSpin->setMaximum(180);
    formLayout->addRow("Angle", angleSpin);


    positionEdit = new Vector3Edit;
    formLayout->addRow("Position", positionEdit);

    rotationEdit = new Vector3Edit;
    formLayout->addRow("Rotation", rotationEdit);

}

TrussEditorWidget::TrussEditorWidget(Truss *t_truss, QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    setLayout(m_impl->formLayout);

    setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    connect(m_impl->nameEdit, &QLineEdit::textEdited, this, &TrussEditorWidget::setName);
    connect(m_impl->beamSpin, &QSpinBox::valueChanged, this, &TrussEditorWidget::setBeams);
    connect(m_impl->segmentLengthSpin, &QDoubleSpinBox::valueChanged, this, &TrussEditorWidget::setSegmentLength);
    connect(m_impl->radiusSpin, &QDoubleSpinBox::valueChanged, this, &TrussEditorWidget::setRadius);
    connect(m_impl->offsetSpin, &QDoubleSpinBox::valueChanged, this, &TrussEditorWidget::setOffset);
    connect(m_impl->lengthSpin, &QDoubleSpinBox::valueChanged, this, &TrussEditorWidget::setLength);
    connect(m_impl->angleSpin, &QDoubleSpinBox::valueChanged, this, &TrussEditorWidget::setAngle);
    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &TrussEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &TrussEditorWidget::setRotation);

    m_impl->truss = t_truss;

    m_impl->nameEdit->setText(t_truss->name());
    m_impl->beamSpin->setValue(t_truss->beams());
    m_impl->segmentLengthSpin->setValue(t_truss->segmentLength());
    m_impl->radiusSpin->setValue(t_truss->radius());
    m_impl->offsetSpin->setValue(t_truss->offset());
    m_impl->lengthSpin->setValue(t_truss->length());
    m_impl->angleSpin->setValue(t_truss->angle());
    m_impl->positionEdit->setValue(t_truss->position());
    m_impl->rotationEdit->setValue(t_truss->rotation());
}

TrussEditorWidget::~TrussEditorWidget()
{
    delete m_impl;
}

void TrussEditorWidget::setName(const QString &t_value)
{
    m_impl->truss->setName(t_value);
}

void TrussEditorWidget::setBeams(int t_value)
{
    m_impl->truss->setBeams(t_value);
}

void TrussEditorWidget::setSegmentLength(float t_value)
{
    m_impl->truss->setSegmentLength(t_value);
}

void TrussEditorWidget::setRadius(float t_value)
{
    m_impl->truss->setRadius(t_value);
}

void TrussEditorWidget::setOffset(float t_value)
{
    m_impl->truss->setOffset(t_value);
}

void TrussEditorWidget::setLength(float t_value)
{
    m_impl->truss->setLength(t_value);
}

void TrussEditorWidget::setAngle(float t_value)
{
    m_impl->truss->setAngle(t_value);
}

void TrussEditorWidget::setPosition(const QVector3D &t_value)
{
    m_impl->truss->setPosition(t_value);
}

void TrussEditorWidget::setRotation(const QVector3D &t_value)
{
    m_impl->truss->setRotation(t_value);
}





class Truss::Impl
{
public:
    float segmentLength = .25;
    float length = 4;
    float angle = 0.0f;
    int beams = 4;
    float offset = .25f;
    float radius = .025f;
};

Truss::Truss() : SceneObject("truss"),m_impl(new Impl)
{

}

Truss::~Truss()
{
    delete m_impl;
}

QWidget *Truss::createEditor()
{
    return new TrussEditorWidget(this);
}

void Truss::setSegmentLength(float t_value)
{
    m_impl->segmentLength = t_value;
    emit metadataChanged();
}

void Truss::setRadius(float t_value)
{
    m_impl->radius = t_value;
    emit metadataChanged();
}

void Truss::setBeams(uint t_beams)
{
    m_impl->beams = t_beams;
    emit metadataChanged();
}

void Truss::setOffset(float t_offset)
{
    m_impl->offset = t_offset;
    emit metadataChanged();
}

void Truss::setLength(float t_length)
{
    m_impl->length = t_length;
    emit metadataChanged();
}

void Truss::setAngle(float t_angle)
{
    m_impl->angle = t_angle;
    emit metadataChanged();
}

float Truss::radius() const
{
    return m_impl->radius;
}

float Truss::segmentLength() const
{
    return m_impl->segmentLength;
}

float Truss::offset() const
{
    return m_impl->offset;
}

float Truss::length() const
{
    return m_impl->length;
}

float Truss::angle() const
{
    return m_impl->angle;
}

uint Truss::beams() const
{
    return m_impl->beams;
}

void Truss::readFromJson(const QJsonObject &t_json)
{
    SceneObject::readFromJson(t_json);

    m_impl->segmentLength = t_json.value("segmentLength").toDouble(m_impl->segmentLength);
    m_impl->length = t_json.value("length").toDouble(m_impl->length);
    m_impl->radius = t_json.value("radius").toDouble(m_impl->radius);
    m_impl->offset = t_json.value("offset").toDouble(m_impl->offset);
    m_impl->angle = t_json.value("angle").toDouble(m_impl->angle);
    m_impl->beams = t_json.value("beams").toInt(m_impl->beams);
}

void Truss::writeToJson(QJsonObject &t_json) const
{
    SceneObject::writeToJson(t_json);
    t_json.insert("segmentLength", m_impl->segmentLength);
    t_json.insert("length", m_impl->length);
    t_json.insert("radius", m_impl->radius);
    t_json.insert("offset", m_impl->offset);
    t_json.insert("angle", m_impl->angle);
    t_json.insert("beams", m_impl->beams);
}

} // namespace photon
