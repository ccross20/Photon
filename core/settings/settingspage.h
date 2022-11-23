#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H
#include <QWidget>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT SettingsPage : public QWidget
{
    Q_OBJECT
public:
    SettingsPage(QWidget *parent = nullptr);
    ~SettingsPage() override;



private:
    class Impl;
    Impl *m_impl;
};

} // namespace deco

#endif // SETTINGSPAGE_H
