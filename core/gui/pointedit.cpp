#include <QDoubleSpinBox>
#include <QVector3D>
#include <QHBoxLayout>
#include "pointedit.h"

namespace photon {

class PointEdit::Impl
{
public:
    QDoubleSpinBox *createSpin();
    QDoubleSpinBox *xSpin;
    QDoubleSpinBox *ySpin;
    QPointF value;
    PointEdit *facade;
};

QDoubleSpinBox *PointEdit::Impl::createSpin()
{
    QDoubleSpinBox *spin = new QDoubleSpinBox;
    spin->setMinimum(-99999);
    spin->setMaximum(99999);
    spin->setDecimals(4);

    connect(spin, &QDoubleSpinBox::valueChanged, facade, &PointEdit::inputChanged);

    return spin;
}

PointEdit::PointEdit(QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    m_impl->facade = this;

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);

    m_impl->xSpin = m_impl->createSpin();
    m_impl->ySpin = m_impl->createSpin();

    hLayout->addWidget(m_impl->xSpin);
    hLayout->addWidget(m_impl->ySpin);

    setLayout(hLayout);
}

PointEdit::~PointEdit()
{
    delete m_impl;
}

void PointEdit::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    if(event->type() == QEvent::EnabledChange)
    {
        m_impl->xSpin->setEnabled(isEnabled());
        m_impl->ySpin->setEnabled(isEnabled());
    }
}

void PointEdit::setValue(const QPointF &t_value)
{
    m_impl->value = t_value;

    m_impl->xSpin->setValue(t_value.x());
    m_impl->ySpin->setValue(t_value.y());
}

QPointF PointEdit::value() const
{
    return m_impl->value;
}

void PointEdit::inputChanged(double)
{
    m_impl->value.setX(m_impl->xSpin->value());
    m_impl->value.setY(m_impl->ySpin->value());

    emit valueChanged(m_impl->value);
}

} // namespace photon
