#ifndef PANEL_P_H
#define PANEL_P_H

#include "panel.h"

class QVBoxLayout;

namespace photon {


class Panel::Impl
{

public:
    Impl(const PanelId &id, Panel *panel);
    ~Impl();

    PanelId id;
    GuiManager *gui;
    QBrush backgroundBrush;
    QVector<QWidget*> windowFrameWidgets;
    QToolBar *toolbar;
    QMenuBar *menubar;
    QVBoxLayout *layout;
    QWidget *content = nullptr;
    Qt::Orientation orientation = Qt::Horizontal;
    QString name;
    QByteArray uniqueId;

    ads::CDockWidget *dockWidget;
    bool isActivated = false;
    bool toolbarIsHidden = true;
    bool scrollIfPossible = true;
    bool hideFrame = false;

protected:
    Panel *m_ext;

private:


};

} // deco namespace

#endif // PANEL_P_H
