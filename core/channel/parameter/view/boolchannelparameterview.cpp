#include <QVBoxLayout>
#include <QCheckBox>
#include "boolchannelparameterview.h"
#include "../boolchannelparameter.h"

namespace photon {


class BoolChannelParameterView::Impl
{
public:
    QCheckBox *checkBox;
};



BoolChannelParameterView::BoolChannelParameterView(BoolChannelParameter *t_param) : ChannelParameterView(t_param),m_impl(new Impl)
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    m_impl->checkBox = new QCheckBox;
    m_impl->checkBox->setChecked(t_param->value().toBool());
    vLayout->addWidget(m_impl->checkBox);
    setLayout(vLayout);

    connect(m_impl->checkBox, &QCheckBox::clicked, this, &BoolChannelParameterView::checkUpdated);
}

BoolChannelParameterView::~BoolChannelParameterView()
{
    delete m_impl;
}

void BoolChannelParameterView::checkUpdated(bool t_value)
{
    parameter()->setValue(t_value);
}

} // namespace photon
