#ifndef PHOTON_APPSETTINGSPAGE_H
#define PHOTON_APPSETTINGSPAGE_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

// Base for a single page in SettingsDialog's page list. Subclasses build their
// own controls and are responsible for reading/writing their own values as the
// user edits them (matching how the rest of the app's QSettings-backed widget
// state already works) - the dialog itself has no Apply/Cancel.
class PHOTONCORE_EXPORT AppSettingsPage : public QWidget
{
    Q_OBJECT
public:
    explicit AppSettingsPage(const QString &t_pageName, QWidget *t_parent = nullptr);

    QString pageName() const;

private:
    QString m_pageName;
};

} // namespace photon

#endif // PHOTON_APPSETTINGSPAGE_H
