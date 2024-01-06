#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QFileDialog>
#include <QSettings>
#include "fixturecollectionpanel_p.h"
#include "fixture/fixture.h"
#include "photoncore.h"
#include "project/project.h"
#include "fixture/fixturecollection.h"
#include "scene/scenegroup.h"
#include "scene/truss.h"
#include "pixel/pixelstrip.h"
#include "scene/sceneiterator.h"
#include "scene/scenearrow.h"

namespace photon {


FixtureCollectionPanel::FixtureCollectionPanel() : Panel("photon.fixture-collection"),m_impl(new Impl)
{
    m_impl->editorWidget = nullptr;
    m_impl->vLayout = new QVBoxLayout;

    setName("Fixtures");

    m_impl->treeView = new QTreeView;
    m_impl->treeView->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    m_impl->treeView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    m_impl->treeView->setDragEnabled(true);
    m_impl->treeView->setDragDropMode(QAbstractItemView::DragDrop);

    m_impl->vLayout->addWidget(m_impl->treeView);

    m_impl->addButton = new QPushButton("Add");
    m_impl->addButton->setEnabled(false);
    m_impl->removeButton = new QPushButton("Remove");
    m_impl->removeButton->setEnabled(false);
    m_impl->duplicateButton = new QPushButton("Duplicate");
    m_impl->duplicateButton->setEnabled(false);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_impl->addButton);
    hLayout->addWidget(m_impl->removeButton);
    hLayout->addWidget(m_impl->duplicateButton);

    m_impl->vLayout->addLayout(hLayout);

    //vLayout->addWidget(m_impl->editorWidget);


    setPanelLayout(m_impl->vLayout);


    connect(m_impl->addButton, &QPushButton::clicked, this, &FixtureCollectionPanel::addClicked);
    connect(m_impl->removeButton, &QPushButton::clicked, this, &FixtureCollectionPanel::removeClicked);
    connect(m_impl->duplicateButton, &QPushButton::clicked, this, &FixtureCollectionPanel::duplicateClicked);
    connect(m_impl->treeView, &QTreeView::doubleClicked, this, &FixtureCollectionPanel::doubleClicked);
}

FixtureCollectionPanel::~FixtureCollectionPanel()
{
    delete m_impl;
}

void FixtureCollectionPanel::selectionMoved(QModelIndexList indices)
{
    m_impl->treeView->selectionModel()->select(indices[0], QItemSelectionModel::ClearAndSelect);
}

void FixtureCollectionPanel::doubleClicked(const QModelIndex &t_index)
{
    Fixture *fixture = static_cast<Fixture*>(t_index.internalPointer());

    if(fixture)
    {
        //photonApp->editFixture(fixture);
    }
}

void FixtureCollectionPanel::addClicked()
{
    QMenu menu;
    menu.addAction("Fixture",[this](){addFixture();});
    menu.addAction("Group",[this](){addGroup();});
    menu.addAction("Truss",[this](){addTruss();});
    menu.addAction("Light Strip",[this](){addLightStrip();});
    menu.addAction("Arrow",[this](){addArrow();});

    menu.exec(m_impl->addButton->mapToGlobal(m_impl->addButton->rect().bottomLeft()));
}

void FixtureCollectionPanel::duplicateClicked()
{
    auto fixtures = SceneIterator::FindMany(photonApp->project()->sceneRoot(),[](SceneObject *t_object, bool *t_continue){
       return dynamic_cast<Fixture*>(t_object);
    });

    int nextDMX = 0;
    for(auto fix : fixtures)
    {
        int end = static_cast<Fixture*>(fix)->dmxOffset() + static_cast<Fixture*>(fix)->dmxSize();

        if(end > nextDMX)
            nextDMX = end;
    }


    auto indicies = m_impl->treeView->selectionModel()->selectedRows();

    QVector<Fixture*> toClone;
    for(auto &index : indicies)
    {
        toClone.append(static_cast<Fixture*>(index.internalPointer()));
    }

    for(auto c : toClone)
    {
        auto newObj = c->clone();
        if(newObj)
        {
            newObj->setParentSceneObject(c->parentSceneObject(),c->index());
        }

        auto fixtures = SceneIterator::FindMany(newObj,[](SceneObject *t_object, bool *t_continue){
           return dynamic_cast<Fixture*>(t_object);
        });
        for(auto sceneObj : fixtures)
        {
            auto fix = static_cast<Fixture*>(sceneObj);
            fix->setDMXOffset(nextDMX);
            nextDMX += fix->dmxSize();
        }
    }
}

void FixtureCollectionPanel::addFixture()
{    
    QString loadPath;
    if(loadPath.isNull())
    {
        QSettings qsettings;
        qsettings.beginGroup("app");
        QString startPath = qsettings.value("definitionpath", QDir::homePath()).toString();
        qsettings.endGroup();

        loadPath = QFileDialog::getOpenFileName(nullptr, "Fixture Definition",
                                            startPath,
                                            "*.json");

        if(loadPath.isNull())
            return;
        qsettings.beginGroup("app");
        qsettings.setValue("definitionpath", QFileInfo(loadPath).path());
        qsettings.endGroup();


        auto fix = new Fixture(loadPath);

        auto sameNameFixtures = photonApp->project()->fixtures()->fixturesWithName(fix->name());

        if(sameNameFixtures.length() > 0)
            fix->setName(fix->name() + " " + QString::number(sameNameFixtures.length() + 1));

        //photonApp->project()->fixtures()->addFixture(fix);
        fix->setParentSceneObject(photonApp->project()->sceneRoot());

    }
}

void FixtureCollectionPanel::addGroup()
{
    auto group = new SceneGroup;
    group->setName("Group");
    group->setParentSceneObject(photonApp->project()->sceneRoot());
}

void FixtureCollectionPanel::addTruss()
{
    auto truss = new Truss;
    truss->setName("Truss");
    truss->setParentSceneObject(photonApp->project()->sceneRoot());
}

void FixtureCollectionPanel::addArrow()
{
    auto arrow = new SceneArrow;
    arrow->setName("Arrow");
    arrow->setParentSceneObject(photonApp->project()->sceneRoot());
}

void FixtureCollectionPanel::addLightStrip()
{
    auto strip = new PixelStrip;
    strip->setName("Pixel Strip");
    strip->setParentSceneObject(photonApp->project()->sceneRoot());
}

void FixtureCollectionPanel::removeClicked()
{
    auto indicies = m_impl->treeView->selectionModel()->selectedRows();

    m_impl->treeView->clearSelection();
    QVector<Fixture*> toDelete;
    for(auto &index : indicies)
    {
        toDelete.append(static_cast<Fixture*>(index.internalPointer()));
    }

    for(auto seq : toDelete)
    {
        seq->setParentSceneObject(nullptr);
        //seq->deleteLater();
    }

}

void FixtureCollectionPanel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    m_impl->removeButton->setEnabled(m_impl->treeView->selectionModel()->hasSelection());
    m_impl->duplicateButton->setEnabled(m_impl->treeView->selectionModel()->hasSelection());

    auto selectedIndices = m_impl->treeView->selectionModel()->selectedIndexes();

    if(m_impl->editorWidget)
        delete m_impl->editorWidget;
    m_impl->editorWidget = nullptr;

    for(auto index : selectedIndices)
    {
        auto sceneObj = static_cast<SceneObject*>(index.internalPointer());
        if(sceneObj)
        {
            m_impl->editorWidget = sceneObj->createEditor();
            m_impl->vLayout->addWidget(m_impl->editorWidget);
            return;
        }
    }
}

void FixtureCollectionPanel::projectDidOpen(photon::Project* project)
{
    m_impl->sceneModel = new SceneModel(project->sceneRoot());
    m_impl->treeView->setModel(m_impl->sceneModel);
    connect(m_impl->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FixtureCollectionPanel::selectionChanged);
    connect(m_impl->sceneModel, &SceneModel::selectionMoved, this, &FixtureCollectionPanel::selectionMoved);

    m_impl->addButton->setEnabled(true);
}

void FixtureCollectionPanel::projectWillClose(photon::Project* project)
{
    disconnect(m_impl->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FixtureCollectionPanel::selectionChanged);
    disconnect(m_impl->sceneModel, &SceneModel::selectionMoved, this, &FixtureCollectionPanel::selectionMoved);
    m_impl->treeView->setModel(nullptr);
    delete m_impl->sceneModel;
    m_impl->sceneModel = nullptr;
    m_impl->addButton->setEnabled(false);
    m_impl->removeButton->setEnabled(false);
    delete m_impl->editorWidget;
}


} // namespace photon
