#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QAction>
#include <QKeyEvent>
#include <QRandomGenerator>

#include "guimanager_p.h"
#include "panel_p.h"

namespace photon {



Panel::Impl::Impl(const PanelId &id, Panel *panel) : id(id),
    gui(nullptr),
    toolbar(new QToolBar()),
    menubar(new QMenuBar(panel)),
    layout(new QVBoxLayout()),
    m_ext(panel)
{
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addWidget(toolbar);
    toolbar->setVisible(!toolbarIsHidden);
    menubar->setNativeMenuBar(false);
    menubar->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    //backgroundBrush = QBrush(core::applicationSetting("deco.settings.ui.color.viewport.bg").value<QColor>());
    //backgroundBrush = QBrush(QColor(QRandomGenerator::global()->bounded(255),QRandomGenerator::global()->bounded(255),QRandomGenerator::global()->bounded(255)));
    backgroundBrush = QBrush(QColor(80,80,80));
}

Panel::Impl::~Impl()
{

}

Panel::Panel(const PanelId &id, QWidget *parent) : QWidget(parent), m_impl(new Panel::Impl(id, this))
{
    setLayout(m_impl->layout);
    setMinimumSize(100,20);
    setObjectName(id);
    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    setProperty("styleClass","panel");
/*
    connect(exoApp,SIGNAL(projectDidOpen(Project*)),this,SLOT(projectDidOpenSlot(Project*)));
    connect(exoApp,SIGNAL(projectWillClose(Project*)),this,SLOT(projectWillCloseSlot(Project*)));
    connect(exoApp->sessions(),SIGNAL(activeAddressChanged(const GlobalAddress &, const GlobalAddress &)),this,SLOT(activeAddressChangedSlot(const GlobalAddress &, const GlobalAddress &)));
    connect(exoApp->sessions(),SIGNAL(activeSceneChanged(ScenePtr, ScenePtr)),this,SLOT(activeSceneChangedSlot(ScenePtr, ScenePtr)));
*/
}

Panel::~Panel()
{
}

PanelId Panel::id() const
{
    return m_impl->id;
}


void Panel::init()
{
}

void Panel::enterEvent(QEnterEvent *event)
{
    QWidget::enterEvent(event);

    //exoApp->gui()->m_impl->activatePanel(this);
}

void Panel::setIsFrameHidden(bool value)
{
    m_impl->hideFrame = value;
}

bool Panel::isFrameHidden() const
{
    return m_impl->hideFrame;
}

void Panel::setOrientation(Qt::Orientation orientation)
{
    m_impl->orientation = orientation;
}

Qt::Orientation Panel::orientation() const
{
    return m_impl->orientation;
}

QVector<QWidget*> Panel::windowFrameWidgets() const
{
    return m_impl->windowFrameWidgets;
}

void Panel::addWindowFrameWidget(QWidget *widget)
{
    if(m_impl->windowFrameWidgets.contains(widget))
        return;
    m_impl->windowFrameWidgets.append(widget);
    emit frameWidgetAdded(widget);
}

void Panel::removeWindowFrameWidget(QWidget *widget)
{
    if(m_impl->windowFrameWidgets.removeOne(widget))
        emit frameWidgetRemoved(widget);
}

void Panel::setPanelLayout(QLayout *layout)
{
    m_impl->layout->addLayout(layout);
}

void Panel::setPanelWidget(QWidget *widget)
{
    m_impl->layout->addWidget(widget);
    m_impl->content = widget;
}

QWidget *Panel::panelWidget() const
{
    return m_impl->content;
}

QString Panel::name() const
{
    if(m_impl->name.isEmpty())
        return id();
    return m_impl->name;
}

void Panel::setName(const QString &t_value)
{
    if(m_impl->name == t_value)
        return;
    m_impl->name = t_value;

    emit nameUpdated(m_impl->name);
}

void Panel::projectDidOpen(Project* project)
{
    //projectDidOpen(project);
}

void Panel::projectWillClose(Project* project)
{
    //projectWillClose(project);
}

void Panel::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void Panel::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void Panel::activate()
{
    m_impl->isActivated = true;

    emit activationToggled(true);
    didActivate();
    update();
}

void Panel::deactivate()
{
    m_impl->isActivated = false;

    didDeactivate();
    emit activationToggled(false);
    update();
}

bool Panel::isActivated() const
{
    return m_impl->isActivated;
}

void Panel::setToolbarIsHidden(bool value)
{
    if(m_impl->toolbarIsHidden == value)
        return;

    m_impl->toolbar->setVisible(!value);
    m_impl->toolbarIsHidden = value;
}

bool Panel::toolbarIsHidden() const
{
    return m_impl->toolbarIsHidden;
}

void Panel::showToolbar()
{
    setToolbarIsHidden(false);
}

void Panel::hideToolbar()
{
    setToolbarIsHidden(true);
}

void Panel::setScrollIfPossible(bool value)
{
    if(m_impl->scrollIfPossible == value)
        return;

    m_impl->scrollIfPossible = value;
}

bool Panel::scrollIfPossible() const
{
    return m_impl->scrollIfPossible;
}

QToolBar *Panel::toolbar() const
{
    return m_impl->toolbar;
}

QMenuBar *Panel::menubar() const
{
    return m_impl->menubar;
}


GuiManager *Panel::gui() const
{
    return m_impl->gui;
}

void Panel::setGui(GuiManager *gui)
{
    m_impl->gui = gui;
}


bool Panel::handleSequence(const QKeySequence &t_sequence)
{
    /*
    if(t_sequence.isEmpty())
        return false;
    int key = CommandManager::keyFromSequence(t_sequence);
    if(key != 0)
    {
        QKeyEvent event{QEvent::KeyPress, key, exoApp->keyboardModifiers()};
        return exoApp->gui()->processKeyEvent(&event);
    }
*/

    return false;
}

void Panel::buildMenu(QMenuBar */*menubar*/)
{

}

void Panel::paintEvent(QPaintEvent *pe)
{
    QPainter p(this);

    p.fillRect(pe->rect(),m_impl->backgroundBrush);
}

bool Panel::panelKeyPress(QKeyEvent *event)
{
    Q_UNUSED(event)
    return false;
}

bool Panel::panelKeyRelease(QKeyEvent *event)
{
    Q_UNUSED(event)
    return false;
}

void Panel::didActivate()
{

}

void Panel::didDeactivate()
{

}


} // namespace exo
