#include <QHBoxLayout>
#include <QVBoxLayout>
#include "pointedit.h"
#include "xypad.h"
#include "view/numberscrubfield.h"

namespace photon {

class PointEdit::Impl
{
public:
    keira::NumberScrubField *xField = nullptr;
    keira::NumberScrubField *yField = nullptr;
    XYPad *pad = nullptr;
    QPointF value;
    bool updating = false;
    PointEdit *facade = nullptr;
};

PointEdit::PointEdit(QWidget *parent)
    : QWidget{parent}, m_impl(new Impl)
{
    m_impl->facade = this;

    m_impl->pad = new XYPad;
    m_impl->pad->setFixedSize(46, 46);

    m_impl->xField = new keira::NumberScrubField;
    m_impl->xField->setDecimals(4);
    m_impl->yField = new keira::NumberScrubField;
    m_impl->yField->setDecimals(4);

    QVBoxLayout *fieldsLayout = new QVBoxLayout;
    fieldsLayout->setContentsMargins(0, 0, 0, 0);
    fieldsLayout->setSpacing(2);
    fieldsLayout->addWidget(m_impl->xField);
    fieldsLayout->addWidget(m_impl->yField);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(4);
    hLayout->addWidget(m_impl->pad);
    hLayout->addLayout(fieldsLayout);

    setLayout(hLayout);

    // Axis fields -> value.
    connect(m_impl->xField, &keira::NumberScrubField::valueChanged, this, [this](double x){
        if(m_impl->updating) return;
        m_impl->value.setX(x);
        m_impl->updating = true;
        m_impl->pad->setValue(m_impl->value);
        m_impl->updating = false;
        emit valueChanged(m_impl->value);
    });
    connect(m_impl->yField, &keira::NumberScrubField::valueChanged, this, [this](double y){
        if(m_impl->updating) return;
        m_impl->value.setY(y);
        m_impl->updating = true;
        m_impl->pad->setValue(m_impl->value);
        m_impl->updating = false;
        emit valueChanged(m_impl->value);
    });

    // Pad -> value.
    connect(m_impl->pad, &XYPad::valueChanged, this, [this](QPointF p){
        if(m_impl->updating) return;
        m_impl->value = p;
        m_impl->updating = true;
        m_impl->xField->setValue(p.x());
        m_impl->yField->setValue(p.y());
        m_impl->updating = false;
        emit valueChanged(m_impl->value);
    });
}

PointEdit::~PointEdit()
{
    delete m_impl;
}

void PointEdit::setRange(const QPointF &minimum, const QPointF &maximum)
{
    m_impl->xField->setRange(minimum.x(), maximum.x());
    m_impl->yField->setRange(minimum.y(), maximum.y());
    m_impl->pad->setRange(minimum, maximum);
}

void PointEdit::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);

    if(event->type() == QEvent::EnabledChange)
    {
        m_impl->xField->setEnabled(isEnabled());
        m_impl->yField->setEnabled(isEnabled());
        m_impl->pad->setEnabled(isEnabled());
    }
}

void PointEdit::setValue(const QPointF &t_value)
{
    m_impl->value = t_value;

    m_impl->updating = true;
    m_impl->xField->setValue(t_value.x());
    m_impl->yField->setValue(t_value.y());
    m_impl->pad->setValue(t_value);
    m_impl->updating = false;
}

QPointF PointEdit::value() const
{
    return m_impl->value;
}

} // namespace photon
