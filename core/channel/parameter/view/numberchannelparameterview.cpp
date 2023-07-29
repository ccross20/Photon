#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include "numberchannelparameterview.h"
#include "../numberchannelparameter.h"

namespace photon {

class NumberChannelParameterView::Impl
{
public:
    QDoubleSpinBox *spinBox;
};

NumberChannelParameterView::NumberChannelParameterView(NumberChannelParameter *t_param) : ChannelParameterView(t_param),m_impl(new Impl)
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    m_impl->spinBox = new QDoubleSpinBox;
    m_impl->spinBox->setMinimum(t_param->minimum());
    m_impl->spinBox->setMaximum(t_param->maximum());
    m_impl->spinBox->setSingleStep(t_param->increment());
    m_impl->spinBox->setReadOnly(t_param->isReadOnly());
    m_impl->spinBox->setValue(t_param->value().toDouble());
    vLayout->addWidget(m_impl->spinBox);
    setLayout(vLayout);

    connect(m_impl->spinBox, &QDoubleSpinBox::valueChanged, this, &NumberChannelParameterView::spinUpdated);
}

NumberChannelParameterView::~NumberChannelParameterView()
{
    delete m_impl;
}

void NumberChannelParameterView::spinUpdated(double t_value)
{
    parameter()->setValue(t_value);
}

} // namespace photon
