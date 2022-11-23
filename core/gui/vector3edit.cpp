#include <QDoubleSpinBox>
#include <QVector3D>
#include <QHBoxLayout>
#include "vector3edit.h"

namespace photon {

class Vector3Edit::Impl
{
public:
    QDoubleSpinBox *createSpin();
    QDoubleSpinBox *xSpin;
    QDoubleSpinBox *ySpin;
    QDoubleSpinBox *zSpin;
    QVector3D value;
    Vector3Edit *facade;
};

QDoubleSpinBox *Vector3Edit::Impl::createSpin()
{
    QDoubleSpinBox *spin = new QDoubleSpinBox;
    spin->setMinimum(-99999);
    spin->setMaximum(99999);
    spin->setDecimals(4);

    connect(spin, &QDoubleSpinBox::valueChanged, facade, &Vector3Edit::inputChanged);

    return spin;
}

Vector3Edit::Vector3Edit(QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    m_impl->facade = this;

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);

    m_impl->xSpin = m_impl->createSpin();
    m_impl->ySpin = m_impl->createSpin();
    m_impl->zSpin = m_impl->createSpin();

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
