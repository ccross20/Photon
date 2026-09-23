#ifndef PHOTON_PROPERTYTABBAR_H
#define PHOTON_PROPERTYTABBAR_H

#include <QWidget>
#include <QVector>
#include "photon-global.h"
#include "propertyaddress.h"

class QHBoxLayout;

namespace photon {

// The Properties panel's tab strip.
//
// There is always exactly one "follow selection" tab, which retargets as the
// user selects things, plus any number of pinned tabs that stay on their
// target. Pinning the follow tab promotes its current target to a pinned tab
// and leaves the follow tab free to keep tracking.
class PHOTONCORE_EXPORT PropertyTabBar : public QWidget
{
    Q_OBJECT
public:
    struct Tab
    {
        PropertyAddress address;
        QString title;
        bool pinned = false;
        bool valid = true;   // target still resolvable; drawn dimmed when not
    };

    explicit PropertyTabBar(QWidget *parent = nullptr);

    // Index 0 is always the follow-selection tab.
    static constexpr int FollowTabIndex = 0;

    void setFollowTab(const PropertyAddress &address, const QString &title, bool valid);
    // Returns the new tab's index, or the existing one if already pinned.
    int addPinnedTab(const PropertyAddress &address, const QString &title, bool valid);
    void removeTab(int index);

    int currentIndex() const { return m_current; }
    void setCurrentIndex(int index);

    const QVector<Tab> &tabs() const { return m_tabs; }
    QVector<Tab> pinnedTabs() const;

    // True when the follow tab's current target is already pinned - the panel
    // uses this to show the pin control as active.
    bool isPinned(const PropertyAddress &address) const;

signals:
    void currentChanged(int index);
    void tabClosed(int index);

private:
    void rebuild();

    QHBoxLayout  *m_layout = nullptr;
    QVector<Tab>  m_tabs;
    int           m_current = 0;
};

} // namespace photon

#endif // PHOTON_PROPERTYTABBAR_H
