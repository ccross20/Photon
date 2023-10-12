#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "pixelstrip.h"
#include "gui/vector3edit.h"

namespace photon {

class PixelStripEditorWidget::Impl
{
public:
    Impl();
    PixelStrip *pixelStrip;
    QFormLayout *formLayout;
    QLineEdit *nameEdit;
    QSpinBox *pixelCountSpin;
    QSpinBox *universeSpin;
    QSpinBox *offsetSpin;
    QDoubleSpinBox *centerSpin;
    QDoubleSpinBox *rotationSpin;
    QDoubleSpinBox *bendSpin;
    QDoubleSpinBox *lengthSpin;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
};

PixelStripEditorWidget::Impl::Impl(){

    formLayout = new QFormLayout;

    nameEdit = new QLineEdit;
    formLayout->addRow("Name", nameEdit);



    pixelCountSpin = new QSpinBox;
    pixelCountSpin->setMinimum(1);
    pixelCountSpin->setMaximum(512);
    formLayout->addRow("Pixel Count", pixelCountSpin);

    universeSpin = new QSpinBox;
    universeSpin->setMinimum(1);
    universeSpin->setMaximum(512);
    formLayout->addRow("Universe", universeSpin);

    offsetSpin = new QSpinBox;
    offsetSpin->setMinimum(0);
    offsetSpin->setMaximum(511);
    formLayout->addRow("DMX Offset", offsetSpin);

    centerSpin = new QDoubleSpinBox;
    centerSpin->setMinimum(0);
    centerSpin->setMaximum(1);
    formLayout->addRow("Center", centerSpin);

    rotationSpin = new QDoubleSpinBox;
    rotationSpin->setMinimum(-360.0);
    rotationSpin->setMaximum(360.0);
    formLayout->addRow("Rotation", rotationSpin);

    bendSpin = new QDoubleSpinBox;
    bendSpin->setMinimum(-180);
    bendSpin->setMaximum(180);
    formLayout->addRow("Bend", bendSpin);

    lengthSpin = new QDoubleSpinBox;
    lengthSpin->setMinimum(.1);
    lengthSpin->setMaximum(20);
    formLayout->addRow("Length", lengthSpin);


    positionEdit = new Vector3Edit;
    formLayout->addRow("Position", positionEdit);

    rotationEdit = new Vector3Edit;
    formLayout->addRow("Rotation", rotationEdit);
}


PixelStripEditorWidget::PixelStripEditorWidget(PixelStrip *t_strip, QWidget *parent): QWidget{parent},m_impl(new Impl)
{
    setLayout(m_impl->formLayout);

    setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    connect(m_impl->nameEdit, &QLineEdit::textEdited, this, &PixelStripEditorWidget::setName);
    connect(m_impl->offsetSpin, &QSpinBox::valueChanged, this, &PixelStripEditorWidget::setDMXOffset);
    connect(m_impl->universeSpin, &QSpinBox::valueChanged, this, &PixelStripEditorWidget::setUniverse);
    connect(m_impl->centerSpin, &QDoubleSpinBox::valueChanged, this, &PixelStripEditorWidget::setCenter);
    connect(m_impl->bendSpin, &QDoubleSpinBox::valueChanged, this, &PixelStripEditorWidget::setBend);
    connect(m_impl->rotationSpin, &QDoubleSpinBox::valueChanged, this, &PixelStripEditorWidget::setAngle);
    connect(m_impl->lengthSpin, &QDoubleSpinBox::valueChanged, this, &PixelStripEditorWidget::setLength);
    connect(m_impl->pixelCountSpin, &QSpinBox::valueChanged, this, &PixelStripEditorWidget::setPixelCount);
    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &PixelStripEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &PixelStripEditorWidget::setRotation);

    m_impl->pixelStrip = t_strip;

    m_impl->nameEdit->setText(t_strip->name());
    m_impl->universeSpin->setValue(t_strip->universe());
    m_impl->bendSpin->setValue(t_strip->bend());
    m_impl->centerSpin->setValue(t_strip->center());
    m_impl->pixelCountSpin->setValue(t_strip->pixelCount());
    m_impl->offsetSpin->setValue(t_strip->dmxOffset());
    m_impl->lengthSpin->setValue(t_strip->length());
    m_impl->rotationSpin->setValue(t_strip->angle());
    m_impl->positionEdit->setValue(t_strip->position());
    m_impl->rotationEdit->setValue(t_strip->rotation());
}

PixelStripEditorWidget::~PixelStripEditorWidget()
{

}

void PixelStripEditorWidget::setName(const QString & t_value)
{
    m_impl->pixelStrip->setName(t_value);
}

void PixelStripEditorWidget::setDMXOffset(int t_value)
{
    m_impl->pixelStrip->setDMXOffset(t_value);
}

void PixelStripEditorWidget::setUniverse(int t_value)
{
    m_impl->pixelStrip->setUniverse(t_value);
}

void PixelStripEditorWidget::setPixelCount(int t_value)
{
    m_impl->pixelStrip->setPixelCount(t_value);
}

void PixelStripEditorWidget::setLength(float t_value)
{
    m_impl->pixelStrip->setLength(t_value);
}

void PixelStripEditorWidget::setCenter(float t_value)
{
    m_impl->pixelStrip->setCenter(t_value);
}

void PixelStripEditorWidget::setAngle(float t_value)
{
    m_impl->pixelStrip->setAngle(t_value);
}

void PixelStripEditorWidget::setBend(float t_value)
{
    m_impl->pixelStrip->setBend(t_value);
}

void PixelStripEditorWidget::setPosition(const QVector3D &t_value)
{
    m_impl->pixelStrip->setPosition(t_value);
}

void PixelStripEditorWidget::setRotation(const QVector3D &t_value)
{
    m_impl->pixelStrip->setRotation(t_value);
}







class PixelStrip::Impl {
public:
    void remakePoints();

    QVector<QPointF> positions;
    double center = .5;
    double angle = 0;
    double bend = 0;
    double length = 1;
    int offset = 0;
    int pixelCount = 100;
    int universe = 1;
};

void PixelStrip::Impl::remakePoints()
{
    double delta = length / (pixelCount - 1);

    QTransform t;
    t.translate(length * center, 0);
    t.rotate(angle);
    t.translate(length * -center, 0);

    positions.resize(pixelCount);
    double position = 0;
    for(auto it = positions.begin(); it != positions.end(); ++it)
    {
        auto &pt = *it;
        auto transformedPt = t.map(QPointF(position, 0.0));

        pt.setX(transformedPt.x());
        pt.setY(transformedPt.y());
        position += delta;
    }
}

PixelStrip::PixelStrip(SceneObject *t_parent) : SceneObject("pixelstrip", t_parent),m_impl(new Impl)
{
    m_impl->remakePoints();
}

PixelStrip::~PixelStrip()
{
    delete m_impl;
}

QWidget *PixelStrip::createEditor()
{
    return new PixelStripEditorWidget(this);
}

double PixelStrip::center() const
{
    return m_impl->center;
}

double PixelStrip::angle() const
{
    return m_impl->angle;
}

double PixelStrip::bend() const
{
    return m_impl->bend;
}

double PixelStrip::length() const
{
    return m_impl->length;
}

int PixelStrip::pixelCount() const
{
    return m_impl->pixelCount;
}

void PixelStrip::setPixelCount(int t_value)
{
    m_impl->pixelCount = t_value;
    m_impl->remakePoints();
    triggerUpdate();
}

void PixelStrip::setCenter(double t_value)
{
    m_impl->center = t_value;
    m_impl->remakePoints();
    triggerUpdate();
}

void PixelStrip::setAngle(double t_value)
{
    m_impl->angle = t_value;
    m_impl->remakePoints();
    triggerUpdate();
}

void PixelStrip::setLength(double t_value)
{
    m_impl->length = t_value;
    m_impl->remakePoints();
    triggerUpdate();
}

void PixelStrip::setBend(double t_value)
{
    m_impl->bend = t_value;
    m_impl->remakePoints();
    triggerUpdate();
}

void PixelStrip::setDMXOffset(int offset)
{
    m_impl->offset = offset;
    m_impl->remakePoints();
    triggerUpdate();
}

void PixelStrip::setUniverse(int universe)
{
    m_impl->universe = universe;
    m_impl->remakePoints();
    triggerUpdate();
}

const QVector<QPointF> &PixelStrip::positions() const
{
    return m_impl->positions;
}

QByteArray PixelStrip::sourceUniqueId() const
{
    return uniqueId();
}

int PixelStrip::dmxOffset() const
{
    return m_impl->offset;
}

int PixelStrip::dmxSize() const
{
    return m_impl->pixelCount * 4;
}

int PixelStrip::universe() const
{
    return m_impl->universe;
}

void PixelStrip::process(ProcessContext &t_context, const QTransform &t_transform) const
{
    PixelSource::process(t_context, t_transform);
}

void PixelStrip::readFromJson(const QJsonObject &t_json)
{
    SceneObject::readFromJson(t_json);

    m_impl->length = t_json.value("length").toDouble(m_impl->length);
    m_impl->center = t_json.value("center").toDouble(m_impl->center);
    m_impl->angle = t_json.value("angle").toDouble(m_impl->angle);
    m_impl->bend = t_json.value("bend").toDouble(m_impl->bend);
    m_impl->offset = t_json.value("offset").toInt(m_impl->offset);
    m_impl->pixelCount = t_json.value("pixelCount").toInt(m_impl->pixelCount);
    m_impl->universe = t_json.value("universe").toInt(m_impl->universe);
}

void PixelStrip::writeToJson(QJsonObject &t_json) const
{
    SceneObject::writeToJson(t_json);

    t_json.insert("length", m_impl->length);
    t_json.insert("center", m_impl->center);
    t_json.insert("angle", m_impl->angle);
    t_json.insert("bend", m_impl->bend);
    t_json.insert("offset", m_impl->offset);
    t_json.insert("pixelCount", m_impl->pixelCount);
    t_json.insert("universe", m_impl->universe);
}

} // namespace photon
