#include <QJsonArray>
#include <QFrame>
#include "propertiespanel_p.h"
#include "project/project.h"
#include "project/projectresource.h"
#include "photoncore.h"
#include "gui/properties/propertycontroller.h"
#include "gui/properties/propertysubject.h"
#include "scene/sceneobject.h"

namespace photon {

const QByteArray PropertiesPanel::Impl::UiStateKey = "propertyTabs";

PropertiesPanel::PropertiesPanel() : Panel("photon.properties"), m_impl(new Impl)
{
    setName("Properties");

    // Every property page is routed through this one panel, so styling scoped
    // to this object name in styles.css reaches all of them without each
    // editor widget carrying its own stylesheet.
    setObjectName("propertiesPanel");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(10, 8, 10, 10);
    layout->setSpacing(6);

    // --- address bar + pin control ---
    QHBoxLayout *addressRow = new QHBoxLayout;
    addressRow->setContentsMargins(0, 0, 0, 0);

    m_impl->addressBar = new PropertyAddressBar;
    addressRow->addWidget(m_impl->addressBar, 1);

    m_impl->pinButton = new QToolButton;
    m_impl->pinButton->setText(QStringLiteral("Pin"));
    m_impl->pinButton->setCheckable(true);
    m_impl->pinButton->setCursor(Qt::PointingHandCursor);
    m_impl->pinButton->setToolTip(QStringLiteral("Pin this page as a tab"));
    m_impl->pinButton->setStyleSheet(
        "QToolButton { border: 1px solid #3a3a3a; border-radius: 3px; padding: 2px 6px; color: #b0b0b0; }"
        "QToolButton:hover { border-color: #666; color: #e0e0e0; }"
        "QToolButton:checked { background: #3daee9; border-color: #3daee9; color: #ffffff; }");
    connect(m_impl->pinButton, &QToolButton::clicked, this, &PropertiesPanel::togglePin);
    addressRow->addWidget(m_impl->pinButton, 0);

    layout->addLayout(addressRow);

    // --- tab strip ---
    m_impl->tabBar = new PropertyTabBar;
    layout->addWidget(m_impl->tabBar);

    QFrame *rule = new QFrame;
    rule->setFrameShape(QFrame::HLine);
    rule->setStyleSheet("color: #333333;");
    layout->addWidget(rule);

    // --- page ---
    // Property pages vary wildly in height (a two-row gizmo vs. a fixture
    // state editor), so the page scrolls rather than squeezing its rows.
    m_impl->scroll = new QScrollArea;
    m_impl->scroll->setWidgetResizable(true);
    m_impl->scroll->setFrameShape(QFrame::NoFrame);
    m_impl->scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(m_impl->scroll, 1);

    m_impl->emptyLabel = new QLabel(QStringLiteral("No selection"));
    m_impl->emptyLabel->setAlignment(Qt::AlignCenter);
    m_impl->emptyLabel->setStyleSheet("color: #888;");
    m_impl->scroll->setWidget(m_impl->emptyLabel);

    setPanelLayout(layout);

    connect(m_impl->addressBar, &PropertyAddressBar::segmentActivated,
            this, &PropertiesPanel::addressSegmentActivated);
    connect(m_impl->tabBar, &PropertyTabBar::currentChanged, this, &PropertiesPanel::tabChanged);
    connect(m_impl->tabBar, &PropertyTabBar::tabClosed, this, &PropertiesPanel::tabClosed);
    connect(PropertyController::instance(), &PropertyController::subjectChanged,
            this, &PropertiesPanel::subjectChanged);

    subjectChanged(PropertyController::instance()->subject());
}

PropertiesPanel::~PropertiesPanel()
{
    delete m_impl;
}

// Swaps in the page for `subject`, or the empty label when there is none.
// Ownership of the page passes to the scroll area, which deletes the previous.
void PropertiesPanel::subjectChanged(PropertySubject *t_subject)
{
    // Only the follow tab tracks the selection; a pinned tab stays put.
    const bool following = m_impl->tabBar->currentIndex() == PropertyTabBar::FollowTabIndex;

    if (t_subject) {
        m_impl->tabBar->setFollowTab(t_subject->address(), t_subject->title(), t_subject->isValid());
        m_impl->pinButton->setChecked(m_impl->tabBar->isPinned(t_subject->address()));
        m_impl->pinButton->setEnabled(true);
    } else {
        m_impl->tabBar->setFollowTab(PropertyAddress(), QString(), false);
        m_impl->pinButton->setChecked(false);
        m_impl->pinButton->setEnabled(false);
    }

    if (following)
        showSubject(t_subject);
}

// Builds and installs the page. Separate from subjectChanged() because tab
// switching shows a subject the controller never emitted.
void PropertiesPanel::showSubject(PropertySubject *t_subject)
{
    QWidget *page = (t_subject && t_subject->isValid()) ? t_subject->createEditor() : nullptr;

    m_impl->addressBar->setAddress(t_subject ? t_subject->address() : PropertyAddress());

    if (page) {
        // setWidget() deletes whatever was there, including the empty label, so
        // it is recreated on the way back out rather than kept alive.
        m_impl->scroll->setWidget(page);
        m_impl->editorWidget = page;
        m_impl->emptyLabel = nullptr;
    } else {
        QLabel *empty = new QLabel(t_subject ? QStringLiteral("Target no longer exists")
                                             : QStringLiteral("No selection"));
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet("color: #888;");
        m_impl->scroll->setWidget(empty);
        m_impl->emptyLabel = empty;
        m_impl->editorWidget = nullptr;
    }
}

void PropertiesPanel::tabChanged(int t_index)
{
    if (t_index == PropertyTabBar::FollowTabIndex) {
        showSubject(PropertyController::instance()->subject());
        return;
    }

    // A pinned tab resolves its own subject from the stored address, which is
    // the same path a tab restored from the project file takes.
    const auto &tabs = m_impl->tabBar->tabs();
    if (t_index < 0 || t_index >= tabs.size())
        return;

    std::unique_ptr<PropertySubject> subject(PropertySubjectFactory::resolve(tabs.at(t_index).address));
    m_impl->addressBar->setAddress(tabs.at(t_index).address);
    showSubject(subject.get());
}

void PropertiesPanel::tabClosed(int t_index)
{
    m_impl->tabBar->removeTab(t_index);
    savePinnedTabs();

    PropertySubject *current = PropertyController::instance()->subject();
    if (current)
        m_impl->pinButton->setChecked(m_impl->tabBar->isPinned(current->address()));
}

void PropertiesPanel::togglePin()
{
    PropertySubject *subject = PropertyController::instance()->subject();
    if (!subject) {
        m_impl->pinButton->setChecked(false);
        return;
    }

    const PropertyAddress address = subject->address();

    if (m_impl->tabBar->isPinned(address)) {
        // Unpin: find and drop that tab.
        const auto &tabs = m_impl->tabBar->tabs();
        for (int i = 0; i < tabs.size(); ++i) {
            if (tabs.at(i).pinned && tabs.at(i).address == address) {
                m_impl->tabBar->removeTab(i);
                break;
            }
        }
        m_impl->pinButton->setChecked(false);
    } else {
        m_impl->tabBar->addPinnedTab(address, subject->title(), subject->isValid());
        m_impl->pinButton->setChecked(true);
    }

    savePinnedTabs();
}

// A breadcrumb was clicked: select that ancestor, which routes back here
// through the controller like any other selection. PropertySubjectFactory::
// resolve() handles every segment kind, including ones that aren't directly
// editable (a graph, a surface) by navigating to whatever represents them -
// the owning routine or node, the surface resource.
void PropertiesPanel::addressSegmentActivated(const PropertyAddress::Segment &t_segment, int)
{
    if (t_segment.id.isEmpty())
        return;   // a fixed root ("Project", "Rig") is a label, not a target

    PropertyAddress ancestor;
    ancestor.append(t_segment.kind, t_segment.id, t_segment.label);

    if (PropertySubject *subject = PropertySubjectFactory::resolve(ancestor))
        PropertyController::instance()->setSubject(subject);
}

void PropertiesPanel::savePinnedTabs()
{
    Project *project = photonApp ? photonApp->project() : nullptr;
    if (!project)
        return;

    QJsonArray array;
    for (const auto &tab : m_impl->tabBar->pinnedTabs()) {
        QJsonObject entry;
        tab.address.writeToJson(entry);
        array.append(entry);
    }

    QJsonObject state;
    state.insert("pinned", array);
    project->setUiState(Impl::UiStateKey, state);
}

void PropertiesPanel::restorePinnedTabs()
{
    Project *project = photonApp ? photonApp->project() : nullptr;
    if (!project)
        return;

    const QJsonArray array = project->uiState(Impl::UiStateKey).value("pinned").toArray();
    for (const auto &value : array) {
        PropertyAddress address;
        address.readFromJson(value.toObject());
        if (address.isEmpty())
            continue;

        // Resolve now purely to label the tab and mark it live; a target that
        // has since gone still gets its tab, shown as unavailable.
        std::unique_ptr<PropertySubject> subject(PropertySubjectFactory::resolve(address));
        const QString title = subject ? subject->title() : address.last().label;
        m_impl->tabBar->addPinnedTab(address, title, subject != nullptr);
    }
}

void PropertiesPanel::projectDidOpen(Project *t_project)
{
    // The project panel and viewports still publish selection as a resource;
    // forward it into the controller so every source lands in one place.
    connect(t_project, &Project::selectedResourceChanged,
            this, &PropertiesPanel::selectedResourceChanged);

    restorePinnedTabs();
    selectedResourceChanged(t_project->selectedResource());
}

void PropertiesPanel::projectWillClose(Project *t_project)
{
    disconnect(t_project, &Project::selectedResourceChanged,
               this, &PropertiesPanel::selectedResourceChanged);

    PropertyController::instance()->clear();
}

void PropertiesPanel::selectedResourceChanged(ProjectResource *t_resource)
{
    PropertyController::instance()->selectResource(t_resource);
}

} // namespace photon
