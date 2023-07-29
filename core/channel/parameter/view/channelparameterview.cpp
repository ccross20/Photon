#include "channelparameterview.h"

namespace photon {

class ChannelParameterView::Impl
{
public:
    ChannelParameter *parameter;
};

ChannelParameterView::ChannelParameterView(ChannelParameter *t_parameter) : QWidget(),m_impl(new Impl)
{
    m_impl->parameter = t_parameter;
}

ChannelParameterView::~ChannelParameterView()
{
    delete m_impl;
}

ChannelParameter *ChannelParameterView::parameter() const
{
    return m_impl->parameter;
}

} // namespace photon
