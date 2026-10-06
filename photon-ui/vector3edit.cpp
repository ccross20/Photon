#include <QDoubleSpinBox>
#include <QVector3D>
#include <QHBoxLayout>
#include "vector3edit.h"

namespace photon {

class Vector3Edit::Impl
{
public:
    QDoubleSpinBox *createSpin(Kind kind);
    QDoubleSpinBox *xSpin;
    QDoubleSpinBox *ySpin;
    QDoubleSpinBox *zSpin;
    QVector3D value;
    Vector3Edit *facade;
};

QDoubleSpinBox *Vector3Edit::Impl::createSpin(Kind kind)
{
    QDoubleSpinBox *spin = new QDoubleSpinBox;
    if(kind == Angle)
    {
        spin->setRange(-360.0, 360.0);
        spin->setSuffix(QStringLiteral("°"));
    }
    else
    {
        spin->setRange(-999.0, 999.0);
        spin->setSuffix(QStringLiteral(" m"));
    }
    spin->setDecimals(2);

    // QAbstractSpinBox sizes itself off the widest text this range/decimals
    // could ever show ("-99999.9999", all 5 integer digits plus sign) - far
    // wider than a real position/rotation value needs, and fine for a field
    // that gets a row to itself, but three of these share one row here
    // (Position, Rotation). setMinimumWidth() alone doesn't help: that's only
    // a floor a layout falls back to when it's compressing widgets to fit,
    // and with room to spare (e.g. inside a scroll area) it just renders
    // each field at its full, uncapped sizeHint() regardless - fixing the
    // width outright is what actually bounds it. Size that fixed width off a
    // realistic value instead of the pathological one, so three fit
    // comfortably without each shrinking to barely more than its buttons.
    const int textWidth = spin->fontMetrics().horizontalAdvance(QStringLiteral("-99.99"));
    const int chrome = spin->minimumSizeHint().width() - spin->fontMetrics().horizontalAdvance(QStringLiteral("-999.99"));
    //spin->setFixedWidth(textWidth + chrome);

    connect(spin, &QDoubleSpinBox::valueChanged, facade, &Vector3Edit::inputChanged);

    return spin;
}

Vector3Edit::Vector3Edit(Kind t_kind, QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    m_impl->facade = this;

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->setSpacing(4);

    m_impl->xSpin = m_impl->createSpin(t_kind);
    m_impl->ySpin = m_impl->createSpin(t_kind);
    m_impl->zSpin = m_impl->createSpin(t_kind);

    hLayout->addWidget(m_impl->xSpin);
    hLayout->addWidget(m_impl->ySpin);
    hLayout->addWidget(m_impl->zSpin);

    setLayout(hLayout);
}

Vector3Edit::~Vector3Edit()
{
    delete m_impl;
}

void Vector3Edit::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    if(event->type() == QEvent::EnabledChange)
    {
        m_impl->xSpin->setEnabled(isEnabled());
        m_impl->ySpin->setEnabled(isEnabled());
        m_impl->zSpin->setEnabled(isEnabled());
    }
}

void Vector3Edit::setValue(const QVector3D &t_value)
{
    m_impl->value = t_value;

    m_impl->xSpin->setValue(t_value.x());
    m_impl->ySpin->setValue(t_value.y());
    m_impl->zSpin->setValue(t_value.z());
}

QVector3D Vector3Edit::value() const
{
    return m_impl->value;
}

void Vector3Edit::inputChanged(double)
{
    m_impl->value.setX(m_impl->xSpin->value());
    m_impl->value.setY(m_impl->ySpin->value());
    m_impl->value.setZ(m_impl->zSpin->value());

    emit valueChanged(m_impl->value);
}

} // namespace photon
