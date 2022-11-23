#ifndef PANEL_H
#define PANEL_H

#include <QWidget>
#include <QToolBar>
#include <QMenuBar>
#include "photon-global.h"
#include "gui/guimanager.h"

namespace photon {


class PanelDockTitle;
class BaseEvent;
class CommandContainer;

class PHOTONCORE_EXPORT Panel : public QWidget
{
    Q_OBJECT
public:
    Panel(const PanelId &panelId, QWidget *parent = nullptr);
    virtual ~Panel() override;

    QByteArray id() const;
    QString name() const;
    void setName(const QString &);

    virtual void init();
    void activate();
    void deactivate();

    QToolBar *toolbar() const;
    QMenuBar *menubar() const;
    void setGui(GuiManager *gui);
    GuiManager *gui() const;
    CommandContainer *commands() const;
    QVector<QWidget*> windowFrameWidgets() const;

    void addWindowFrameWidget(QWidget *widget);
    void removeWindowFrameWidget(QWidget *widget);
    void setToolbarIsHidden(bool value);
    void setScrollIfPossible(bool value);
    void setIsFrameHidden(bool value);
    void showToolbar();
    void hideToolbar();
    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;
    virtual bool handleSequence(const QKeySequence &sequence);


    bool toolbarIsHidden() const;
    bool scrollIfPossible() const;
    bool isActivated() const;
    bool isFrameHidden() const;

    void setPanelWidget(QWidget *widget);
    void setPanelLayout(QLayout *layout);
    QWidget *panelWidget() const;

    class Impl;

signals:
    void activationToggled(bool value);
    void frameWidgetAdded(QWidget *widget);
    void frameWidgetRemoved(QWidget *widget);
    void menubarUpdated();
    void nameUpdated(const QString &);

protected:

    virtual void paintEvent(QPaintEvent *pe) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual bool panelKeyPress(QKeyEvent *event);
    virtual bool panelKeyRelease(QKeyEvent *event);
    virtual void buildMenu(QMenuBar *menubar);
    virtual void enterEvent(QEnterEvent *event) override;
    virtual void didActivate();
    virtual void didDeactivate();
    virtual void projectDidOpen(photon::Project* project);
    virtual void projectWillClose(photon::Project* project);

private:
    QScopedPointer<Impl> const m_impl;

    friend class PanelDockTitle;
    friend class GuiManager::Impl;
    friend class GuiManager;
    friend class CommandManagerPrivate;
};

} // namespace exo

#endif // PANEL_H
