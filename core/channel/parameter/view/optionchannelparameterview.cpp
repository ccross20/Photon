#include <QComboBox>
#include <QVBoxLayout>
#include "optionchannelparameterview.h"
#include "../optionchannelparameter.h"

namespace photon {

class OptionChannelParameterView::Impl
{
public:
    QComboBox *combo;
};

OptionChannelParameterView::OptionChannelParameterView(OptionChannelParameter *t_param) : ChannelParameterView(t_param),m_impl(new Impl)
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    m_impl->combo = new QComboBox;
    m_impl->combo->addItems(t_param->options());
    m_impl->combo->setEditable(!t_param->isReadOnly());
    m_impl->combo->setCurrentIndex(t_param->value().toInt());
    vLayout->addWidget(m_impl->combo);
    setLayout(vLayout);

    connect(m_impl->combo, &QComboBox::currentIndexChanged, this, &OptionChannelParameterView::indexUpdated);

}

OptionChannelParameterView::~OptionChannelParameterView()
{
    delete m_impl;
}

void OptionChannelParameterView::indexUpdated(int t_index)
{
    parameter()->setValue(t_index);
}

} // namespace photon
