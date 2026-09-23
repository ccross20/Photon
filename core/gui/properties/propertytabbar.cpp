#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include "propertytabbar.h"

namespace photon {

namespace {

const char *kTabStyle =
    "QPushButton { border: none; border-bottom: 2px solid transparent; padding: 4px 8px;"
    " color: #9a9a9a; background: transparent; }"
    "QPushButton:hover { color: #e0e0e0; }"
    "QPushButton:checked { color: #ffffff; border-bottom: 2px solid #3daee9; }";

const char *kCloseStyle =
    "QPushButton { border: none; padding: 0px 4px; color: #777; background: transparent; }"
    "QPushButton:hover { color: #ff8080; }";

} // namespace

PropertyTabBar::PropertyTabBar(QWidget *t_parent) : QWidget(t_parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(2);

    // The follow tab always exists, even with nothing selected.
    m_tabs.append({ PropertyAddress(), QStringLiteral("Selection"), false, false });
    rebuild();
}

void PropertyTabBar::setFollowTab(const PropertyAddress &t_address, const QString &t_title, bool t_valid)
{
    Tab &tab = m_tabs[FollowTabIndex];
    tab.address = t_address;
    tab.title = t_title.isEmpty() ? QStringLiteral("Selection") : t_title;
    tab.valid = t_valid;
    rebuild();
}

int PropertyTabBar::addPinnedTab(const PropertyAddress &t_address, const QString &t_title, bool t_valid)
{
    for (int i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs.at(i).pinned && m_tabs.at(i).address == t_address)
            return i;   // already pinned; don't duplicate
    }

    m_tabs.append({ t_address, t_title, true, t_valid });
    rebuild();
    return m_tabs.size() - 1;
}

void PropertyTabBar::removeTab(int t_index)
{
    // The follow tab is structural and cannot be closed.
    if (t_index <= FollowTabIndex || t_index >= m_tabs.size())
        return;

    m_tabs.remove(t_index);
    if (m_current >= m_tabs.size())
        m_current = m_tabs.size() - 1;
    else if (m_current > t_index)
        --m_current;

    rebuild();
    emit currentChanged(m_current);
}

void PropertyTabBar::setCurrentIndex(int t_index)
{
    if (t_index < 0 || t_index >= m_tabs.size() || t_index == m_current)
        return;
    m_current = t_index;
    rebuild();
    emit currentChanged(m_current);
}

QVector<PropertyTabBar::Tab> PropertyTabBar::pinnedTabs() const
{
    QVector<Tab> pinned;
    for (const Tab &t : m_tabs)
        if (t.pinned)
            pinned.append(t);
    return pinned;
}

bool PropertyTabBar::isPinned(const PropertyAddress &t_address) const
{
    for (const Tab &t : m_tabs)
        if (t.pinned && t.address == t_address)
            return true;
    return false;
}

void PropertyTabBar::rebuild()
{
    while (QLayoutItem *item = m_layout->takeAt(0)) {
        if (QWidget *w = item->widget())
            w->deleteLater();
        delete item;
    }

    for (int i = 0; i < m_tabs.size(); ++i) {
        const Tab &tab = m_tabs.at(i);

        QPushButton *button = new QPushButton(tab.title);
        button->setCheckable(true);
        button->setChecked(i == m_current);
        button->setCursor(Qt::PointingHandCursor);
        button->setStyleSheet(kTabStyle);
        // A pinned tab whose target is gone stays put (the user asked for it)
        // but reads as unavailable rather than silently showing nothing.
        if (!tab.valid) {
            button->setEnabled(tab.pinned);
            button->setToolTip(QStringLiteral("Target no longer exists"));
        } else {
            button->setToolTip(tab.address.toDisplayString());
        }
        connect(button, &QPushButton::clicked, this, [this, i]() { setCurrentIndex(i); });
        m_layout->addWidget(button);

        if (tab.pinned) {
            QPushButton *close = new QPushButton(QStringLiteral("×"));
            close->setFixedWidth(16);
            close->setCursor(Qt::PointingHandCursor);
            close->setStyleSheet(kCloseStyle);
            connect(close, &QPushButton::clicked, this, [this, i]() { emit tabClosed(i); });
            m_layout->addWidget(close);
        }
    }

    m_layout->addStretch(1);
}

} // namespace photon
