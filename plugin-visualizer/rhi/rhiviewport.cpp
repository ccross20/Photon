#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QButtonGroup>
#include <QLabel>
#include "rhiviewport.h"
#include "rhiwindow.h"

namespace photon {

static QToolButton *makeToolBtn(const QString &text, const QString &tip)
{
    auto *btn = new QToolButton;
    btn->setText(text);
    btn->setToolTip(tip);
    btn->setCheckable(true);
    btn->setAutoRaise(true);
    btn->setMinimumWidth(52);
    return btn;
}

RhiViewport::RhiViewport(QWidget *parent)
    : QWidget(parent)
{
    m_window = new RhiWindow;

    connect(m_window, &RhiWindow::selectionChanged, this, &RhiViewport::selectionChanged);
    connect(m_window, &RhiWindow::gizmoModeChanged, this, &RhiViewport::onGizmoModeChanged);

    // ── Toolbar ──────────────────────────────────────────────────────────────
    QWidget *toolbar = new QWidget;
    toolbar->setFixedHeight(30);
    toolbar->setStyleSheet(
        "QWidget { background: #2b2b2b; }"
        "QToolButton {"
        "  background: #3c3c3c; border: 1px solid #555; border-radius: 3px;"
        "  color: #ccc; padding: 2px 8px; min-width: 50px; }"
        "QToolButton:checked {"
        "  background: #4a6faf; border-color: #6a8fcf; color: #fff; }"
        "QToolButton:hover:!checked { background: #4a4a4a; }"
    );

    auto *selectBtn = makeToolBtn("Select",  "Select objects  [Q]");
    auto *moveBtn   = makeToolBtn("Move",    "Translate gizmo  [W]");
    auto *rotateBtn = makeToolBtn("Rotate",  "Rotate gizmo  [E]");

    selectBtn->setChecked(true); // default: Select mode (no gizmo)

    m_modeGroup = new QButtonGroup(this);
    m_modeGroup->setExclusive(true);
    m_modeGroup->addButton(selectBtn, static_cast<int>(RhiGizmo::None));
    m_modeGroup->addButton(moveBtn,   static_cast<int>(RhiGizmo::Translate));
    m_modeGroup->addButton(rotateBtn, static_cast<int>(RhiGizmo::Rotate));

    m_spaceBtn = makeToolBtn("Global", "Toggle world / local transform axes");
    m_spaceBtn->setChecked(false);

    connect(m_modeGroup, &QButtonGroup::idClicked, m_window,
            [this](int id) { m_window->setGizmoMode(static_cast<RhiGizmo::Mode>(id)); });

    connect(m_spaceBtn, &QToolButton::toggled, this, [this](bool local) {
        m_spaceBtn->setText(local ? "Local" : "Global");
        m_window->setGizmoSpace(local ? RhiGizmo::Local : RhiGizmo::Global);
    });

    QHBoxLayout *tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(4, 2, 4, 2);
    tbLayout->setSpacing(4);
    tbLayout->addWidget(selectBtn);
    tbLayout->addWidget(moveBtn);
    tbLayout->addWidget(rotateBtn);
    tbLayout->addSpacing(12);
    tbLayout->addWidget(m_spaceBtn);
    tbLayout->addStretch();

    // ── Main layout ──────────────────────────────────────────────────────────
    QWidget *container = QWidget::createWindowContainer(m_window, this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(toolbar);
    layout->addWidget(container);
}

void RhiViewport::setSceneRoot(SceneObject *root)
{
    m_window->setSceneRoot(root);
}

void RhiViewport::setSelectedSceneObject(SceneObject *obj)
{
    m_window->setSelectedSceneObject(obj);
}

void RhiViewport::onGizmoModeChanged(RhiGizmo::Mode mode)
{
    // Keep buttons in sync when keyboard shortcuts change the mode.
    QAbstractButton *btn = m_modeGroup->button(static_cast<int>(mode));
    if (btn)
        btn->setChecked(true);
}

} // namespace photon
