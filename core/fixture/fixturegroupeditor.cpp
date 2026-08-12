#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include "fixturegroupeditor.h"
#include "fixturegroup.h"
#include "fixture/fixture.h"
#include "gui/resourceeditorwidget.h"
#include "gui/tag/tageditorwidget.h"
#include "photoncore.h"
#include "project/project.h"
#include "scene/scenezone.h"

namespace photon {

class FixtureGroupEditor::Impl
{
public:
    FixtureGroup *group = nullptr;
    QLineEdit *typeEdit = nullptr;
    TagEditorWidget *tagsEditor = nullptr;
    QComboBox *zoneCombo = nullptr;
    QLabel *matchLabel = nullptr;
};

FixtureGroupEditor::FixtureGroupEditor(FixtureGroup *t_group, QWidget *parent)
    : QWidget{parent}, m_impl(new Impl)
{
    m_impl->group = t_group;

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Name + tags, same as every other resource.
    layout->addWidget(new ResourceEditorWidget(t_group));

    auto *queryBox = new QGroupBox("Fixtures");
    auto *form = new QFormLayout(queryBox);

    m_impl->typeEdit = new QLineEdit;
    m_impl->typeEdit->setPlaceholderText("e.g. mover, par (matches type / category)");

    // A different list from the group's own tags above - this is the set of
    // tags a fixture must ALL carry to belong to the group - but it's the
    // same tag vocabulary/picker/coloring, so it looks and behaves identically.
    m_impl->tagsEditor = new TagEditorWidget(
        [this](){ return m_impl->group ? m_impl->group->query().tags : QStringList(); },
        [this](const QStringList &tags){
            if(!m_impl->group)
                return;
            FixtureQuery query = m_impl->group->query();
            query.tags = tags;
            m_impl->group->setQuery(query);
            updateMatchCount();
        },
        [](){ return photonApp->project() ? photonApp->project()->allTags() : QStringList(); });

    m_impl->zoneCombo = new QComboBox;
    m_impl->zoneCombo->addItem("(none)", "");
    for(const QString &zone : SceneZone::zoneNames(photonApp->project()))
        m_impl->zoneCombo->addItem(zone, zone);

    const FixtureQuery &query = t_group->query();
    m_impl->typeEdit->setText(query.type);
    m_impl->zoneCombo->setCurrentIndex(qMax(0, m_impl->zoneCombo->findData(query.zone)));

    form->addRow("Type", m_impl->typeEdit);
    form->addRow("Tags", m_impl->tagsEditor);
    form->addRow("Zone", m_impl->zoneCombo);

    m_impl->matchLabel = new QLabel;
    m_impl->matchLabel->setStyleSheet("color: #888;");
    form->addRow(QString(), m_impl->matchLabel);

    layout->addWidget(queryBox);
    layout->addStretch(1);

    connect(m_impl->typeEdit, &QLineEdit::editingFinished, this, &FixtureGroupEditor::queryEdited);
    connect(m_impl->zoneCombo, &QComboBox::currentIndexChanged, this, &FixtureGroupEditor::queryEdited);

    updateMatchCount();
}

FixtureGroupEditor::~FixtureGroupEditor()
{
    delete m_impl;
}

void FixtureGroupEditor::queryEdited()
{
    if(!m_impl->group)
        return;

    // Tags are edited (and committed) independently through tagsEditor's own
    // get/set callbacks - only carry the current tags through here rather
    // than overwriting them from a text field that no longer exists.
    FixtureQuery query = m_impl->group->query();
    query.type = m_impl->typeEdit->text().trimmed();
    query.zone = m_impl->zoneCombo->currentData().toString();

    m_impl->group->setQuery(query);
    updateMatchCount();
}

void FixtureGroupEditor::updateMatchCount()
{
    if(!m_impl->group || !photonApp->project())
        return;

    // An empty query matches everything, which is easy to do by accident -
    // say so rather than showing a bare count.
    if(m_impl->group->query().isEmpty())
    {
        m_impl->matchLabel->setText("No filters set — matches every fixture.");
        return;
    }

    const int count = m_impl->group->query().resolve(photonApp->project()).size();
    m_impl->matchLabel->setText(count == 1 ? "Matches 1 fixture." : QString("Matches %1 fixtures.").arg(count));
}

} // namespace photon
