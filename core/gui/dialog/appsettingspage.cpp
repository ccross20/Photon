#include "appsettingspage.h"

namespace photon {

AppSettingsPage::AppSettingsPage(const QString &t_pageName, QWidget *t_parent)
    : QWidget(t_parent), m_pageName(t_pageName)
{
}

QString AppSettingsPage::pageName() const
{
    return m_pageName;
}

} // namespace photon
