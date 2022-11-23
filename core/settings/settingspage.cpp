#include <QFormLayout>
#include "settingspage.h"

namespace photon {

class SettingsPage::Impl
{
public:
    Impl();
    QFormLayout *layout;
};

SettingsPage::Impl::Impl():layout(new QFormLayout())
{

}

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent),m_impl(new Impl())
{
    m_impl->layout->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::AllNonFixedFieldsGrow);
    setLayout(m_impl->layout);
}

SettingsPage::~SettingsPage()
{
    delete m_impl;
}

} // namespace deco
