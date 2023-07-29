#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include "point2channelparameterview.h"
#include "../point2channelparameter.h"

namespace photon {

class Point2ChannelParameterView::Impl
{
public:
    QDoubleSpinBox *spinXBox;
    QDoubleSpinBox *spinYBox;
};

Point2ChannelParameterView::Point2ChannelParameterView(Point2ChannelParameter *t_param) : ChannelParameterView(t_param),m_impl(new Impl)
{

    QPointF val = t_param->value().value<QPointF>();

    QHBoxLayout *hLayout = new QHBoxLayout;
    m_impl->spinXBox = new QDoubleSpinBox;
    m_impl->spinXBox->setMinimum(t_param->minimum().x());
    m_impl->spinXBox->setMaximum(t_param->maximum().x());
    m_impl->spinXBox->setSingleStep(t_param->increment());
    m_impl->spinXBox->setReadOnly(t_param->isReadOnly());
    m_impl->spinXBox->setValue(val.x());
    hLayout->addWidget(m_impl->spinXBox);


    m_impl->spinYBox = new QDoubleSpinBox;
    m_impl->spinYBox->setMinimum(t_param->minimum().y());
    m_impl->spinYBox->setMaximum(t_param->maximum().y());
    m_impl->spinYBox->setSingleStep(t_param->increment());
    m_impl->spinYBox->setReadOnly(t_param->isReadOnly());
    m_impl->spinYBox->setValue(val.y());
    hLayout->addWidget(m_impl->spinYBox);


    setLayout(hLayout);

    connect(m_impl->spinXBox, &QDoubleSpinBox::valueChanged, this, &Point2ChannelParameterView::spinXUpdated);
    connect(m_impl->spinYBox, &QDoubleSpinBox::valueChanged, this, &Point2ChannelParameterView::spinYUpdated);
}

Point2ChannelParameterView::~Point2ChannelParameterView()
{
    delete m_impl;
}

void Point2ChannelParameterView::spinXUpdated(double t_value)
{
    QPointF val = parameter()->value().value<QPointF>();
    val.setX(t_value);

    parameter()->setValue(val);
}

void Point2ChannelParameterView::spinYUpdated(double t_value)
{
    QPointF val = parameter()->value().value<QPointF>();
    val.setY(t_value);

    parameter()->setValue(val);
}

} // namespace photon
