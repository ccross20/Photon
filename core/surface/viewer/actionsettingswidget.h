#ifndef ACTIONSETTINGSWIDGET_H
#define ACTIONSETTINGSWIDGET_H

#include <QWidget>
#include "surface/surfaceaction.h"

namespace photon {

class ActionSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ActionSettingsWidget(SurfaceAction *t_action, QWidget *parent = nullptr);

signals:
};

} // namespace photon

#endif // ACTIONSETTINGSWIDGET_H
