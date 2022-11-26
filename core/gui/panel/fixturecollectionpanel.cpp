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



namespace photon {


FixtureCollectionModel::FixtureCollectionModel(FixtureCollection *t_collection) : QAbstractItemModel(),m_collection(t_collection)
{
    connect(m_collection, &FixtureCollection::fixtureWillBeAdded, this, &FixtureCollectionModel::fixtureWillBeAdded);
    connect(m_collection, &FixtureCollection::fixtureWasAdded, this, &FixtureCollectionModel::fixtureWasAdded);
    connect(m_collection, &FixtureCollection::fixtureWillBeRemoved, this, &FixtureCollectionModel::fixtureWillBeRemoved);
    connect(m_collection, &FixtureCollection::fixtureWasRemoved, this, &FixtureCollectionModel::fixtureWasRemoved);
}

void FixtureCollectionModel::fixtureWillBeAdded(photon::Fixture *, int t_index)
{
    beginInsertRows(QModelIndex(), t_index, t_index);
}

void FixtureCollectionModel::fixtureWasAdded(photon::Fixture *)
{
    endInsertRows();
}

void FixtureCollectionModel::fixtureWillBeRemoved(photon::Fixture *, int t_index)
{
    beginRemoveRows(QModelIndex(), t_index, t_index);
}

void FixtureCollectionModel::fixtureWasRemoved(photon::Fixture *)
{
    endRemoveRows();
}

int FixtureCollectionModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

 QModelIndex FixtureCollectionModel::index(int row, int column, const QModelIndex &parent) const
 {
     return createIndex(row, column, m_collection->fixtureAtIndex(row));
 }

 QModelIndex FixtureCollectionModel::parent(const QModelIndex &index) const
 {
     return QModelIndex();
 }

Qt::ItemFlags FixtureCollectionModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant FixtureCollectionModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
        default:
        return QVariant();

    case Qt::DisplayRole:
        return m_collection->fixtureAtIndex(index.row())->name();

    }
}

QVariant FixtureCollectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch(role)
    {
        default:
            return QVariant();

        case Qt::DisplayRole:
            return "Name";

    }
}

int FixtureCollectionModel::rowCount(const QModelIndex &) const
{
    return m_collection->fixtureCount();
}

FixtureCollectionPanel::FixtureCollectionPanel() : Panel("photon.fixture-collection"),m_impl(new Impl)
{
    m_impl->editorWidget = new FixtureEditorWidget;
    QVBoxLayout *vLayout = new QVBoxLayout;

    setName("Fixtures");

    m_impl->listView = new QListView;
    m_impl->listView->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    m_impl->listView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

    vLayout->addWidget(m_impl->listView);

    m_impl->addButton = new QPushButton("Add");
    m_impl->addButton->setEnabled(false);
    m_impl->removeButton = new QPushButton("Remove");
    m_impl->removeButton->setEnabled(false);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_impl->addButton);
    hLayout->addWidget(m_impl->removeButton);

    vLayout->addLayout(hLayout);

    vLayout->addWidget(m_impl->editorWidget);


    setPanelLayout(vLayout);


    connect(m_impl->addButton, &QPushButton::clicked, this, &FixtureCollectionPanel::addClicked);
    connect(m_impl->removeButton, &QPushButton::clicked, this, &FixtureCollectionPanel::removeClicked);
    connect(m_impl->listView, &QListView::doubleClicked, this, &FixtureCollectionPanel::doubleClicked);
}

FixtureCollectionPanel::~FixtureCollectionPanel()
{
    delete m_impl;
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

        photonApp->project()->fixtures()->addFixture(fix);
    }
}

void FixtureCollectionPanel::removeClicked()
{
    auto indicies = m_impl->listView->selectionModel()->selectedRows();

    QVector<Fixture*> toDelete;
    for(auto &index : indicies)
    {
        toDelete.append(static_cast<Fixture*>(index.internalPointer()));
    }

    for(auto seq : toDelete)
    {
        photonApp->project()->fixtures()->removeFixture(seq);
    }

}

void FixtureCollectionPanel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    m_impl->removeButton->setEnabled(m_impl->listView->selectionModel()->hasSelection());

    auto selectedIndices = m_impl->listView->selectionModel()->selectedIndexes();

    QVector<Fixture*> fixtures;

    for(auto index : selectedIndices)
    {
        fixtures.append(static_cast<Fixture*>(index.internalPointer()));
    }

    m_impl->editorWidget->setFixtures(fixtures);
}

void FixtureCollectionPanel::projectDidOpen(photon::Project* project)
{
    FixtureCollectionModel *model = new FixtureCollectionModel(project->fixtures());
    m_impl->listView->setModel(model);
    connect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FixtureCollectionPanel::selectionChanged);

    m_impl->addButton->setEnabled(true);
}

void FixtureCollectionPanel::projectWillClose(photon::Project* project)
{
    disconnect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FixtureCollectionPanel::selectionChanged);
    m_impl->listView->setModel(nullptr);
    m_impl->addButton->setEnabled(false);
    m_impl->removeButton->setEnabled(false);
    m_impl->editorWidget->setFixtures(QVector<Fixture*>{});
}


} // namespace photon
