#include <QHBoxLayout>
#include "colorchannelparameterview.h"
#include "gui/color/colorwheelswatch.h"
#include "../colorchannelparameter.h"

namespace photon {

class ColorChannelParameterView::Impl
{
public:
    ColorWheelSwatch *swatch;
};


ColorChannelParameterView::ColorChannelParameterView(ColorChannelParameter *t_param) : ChannelParameterView(t_param),m_impl(new Impl)
{

    QColor val = t_param->value().value<QColor>();

    QHBoxLayout *hLayout = new QHBoxLayout;
    m_impl->swatch = new ColorWheelSwatch;
    m_impl->swatch->setColor(val);
    hLayout->addWidget(m_impl->swatch);


    setLayout(hLayout);

    connect(m_impl->swatch, &ColorWheelSwatch::colorChanged, this, &ColorChannelParameterView::colorUpdated);
}

ColorChannelParameterView::~ColorChannelParameterView()
{
    delete m_impl;
}

void ColorChannelParameterView::colorUpdated(const QColor &t_value)
{
    parameter()->setValue(t_value);
}

} // namespace photon
