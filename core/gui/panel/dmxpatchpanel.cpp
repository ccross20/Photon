#include <QInputDialog>
#include "dmxpatchpanel_p.h"
#include "project/project.h"
#include "scene/sceneobject.h"
#include "fixture/fixture.h"
#include "graph/bus/busgraph.h"

namespace photon {

DMXPatchPanel::DMXPatchPanel() : Panel("photon.dmx-patch"), m_impl(new Impl)
{
    setName("DMX Patch");

    m_impl->layout = new QVBoxLayout;

    m_impl->toolbarLayout = new QHBoxLayout;
    m_impl->universeLabel = new QLabel("Universe");
    m_impl->universeSpin = new QSpinBox;
    m_impl->universeSpin->setMinimum(1);
    m_impl->universeSpin->setMaximum(9999);
    m_impl->toolbarLayout->addWidget(m_impl->universeLabel);
    m_impl->toolbarLayout->addWidget(m_impl->universeSpin);

    m_impl->moveToUniverseButton = new QPushButton("Move to Universe");
    m_impl->toolbarLayout->addWidget(m_impl->moveToUniverseButton);

    m_impl->identifyButton = new QPushButton("Identify");
    m_impl->identifyButton->setCheckable(true);
    m_impl->toolbarLayout->addWidget(m_impl->identifyButton);

    m_impl->toolbarLayout->addStretch(1);
    m_impl->layout->addLayout(m_impl->toolbarLayout);

    m_impl->grid = new DMXPatchGrid;

    m_impl->scrollArea = new QScrollArea;
    m_impl->scrollArea->setWidget(m_impl->grid);
    m_impl->scrollArea->setWidgetResizable(false);
    m_impl->layout->addWidget(m_impl->scrollArea);

    connect(m_impl->universeSpin, &QSpinBox::valueChanged, m_impl->grid, &DMXPatchGrid::setUniverse);
    connect(m_impl->moveToUniverseButton, &QPushButton::clicked, this, [this]() {
        const auto selected = m_impl->grid->selectedFixtures();
        if(selected.isEmpty())
            return;

        bool ok = false;
        const int universe = QInputDialog::getInt(this, "Move to Universe", "Universe:",
                                                    m_impl->universeSpin->value(), 1, 9999, 1, &ok);
        if(!ok)
            return;

        for(auto *fixture : selected)
            fixture->setUniverse(universe);

        m_impl->universeSpin->setValue(universe);
    });
    connect(m_impl->identifyButton, &QPushButton::toggled, this, [this](bool checked) {
        if(!m_impl->identifyNode)
            return;

        m_impl->identifyNode->setIdentifyEnabled(checked);
        if(checked)
        {
            auto *fixture = m_impl->grid->selectedFixture();
            m_impl->identifyNode->setIdentifiedFixture(fixture ? fixture->uniqueId() : QByteArray());
        }
    });

    setPanelLayout(m_impl->layout);
}

DMXPatchPanel::~DMXPatchPanel()
{
    delete m_impl;
}

void DMXPatchPanel::projectDidOpen(Project *project)
{
    m_impl->grid->setProject(project);
    m_impl->grid->setSelectedFixtures(project->selectedSceneObjects());

    // The bus graph is rebuilt fresh on every project load, so the node
    // pointer must be re-resolved each time rather than cached across projects.
    m_impl->identifyNode = dynamic_cast<IdentifyFixtureNode*>(project->bus()->findNode("Identify"));
    if(m_impl->identifyNode)
        m_impl->identifyNode->setIdentifyEnabled(false);
    m_impl->identifyButton->blockSignals(true);
    m_impl->identifyButton->setChecked(false);
    m_impl->identifyButton->blockSignals(false);

    auto *grid = m_impl->grid;
    connect(project->sceneRoot(), &SceneObject::descendantAdded, grid, [grid](SceneObject*){ grid->refreshFixtures(); });
    connect(project->sceneRoot(), &SceneObject::descendantRemoved, grid, [grid](SceneObject*){ grid->refreshFixtures(); });
    connect(project->sceneRoot(), &SceneObject::descendantModified, grid, [grid](SceneObject*){ grid->refreshFixtures(); });
    connect(project, &Project::selectedSceneObjectsChanged, grid, &DMXPatchGrid::setSelectedFixtures);

    connect(project, &Project::selectedSceneObjectChanged, this, [this](SceneObject *obj) {
        if(!m_impl->identifyNode || !m_impl->identifyButton->isChecked())
            return;
        auto *fixture = dynamic_cast<Fixture*>(obj);
        m_impl->identifyNode->setIdentifiedFixture(fixture ? fixture->uniqueId() : QByteArray());
    });
}

void DMXPatchPanel::projectWillClose(Project *project)
{
    disconnect(project, nullptr, m_impl->grid, nullptr);
    disconnect(project, nullptr, this, nullptr);
    disconnect(project->sceneRoot(), nullptr, m_impl->grid, nullptr);
    m_impl->grid->setProject(nullptr);
    m_impl->grid->setSelectedFixtures({});

    if(m_impl->identifyNode)
        m_impl->identifyNode->setIdentifyEnabled(false);
    m_impl->identifyNode = nullptr;
    m_impl->identifyButton->blockSignals(true);
    m_impl->identifyButton->setChecked(false);
    m_impl->identifyButton->blockSignals(false);
}

} // namespace photon
