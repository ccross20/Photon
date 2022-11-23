#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include "routinecollectionpanel_p.h"
#include "routine/routine.h"
#include "photoncore.h"
#include "project/project.h"
#include "routine/routinecollection.h"



namespace photon {


RoutineCollectionModel::RoutineCollectionModel(RoutineCollection *t_collection) : QAbstractItemModel(),m_collection(t_collection)
{
    connect(m_collection, &RoutineCollection::routineWillBeAdded, this, &RoutineCollectionModel::routineWillBeAdded);
    connect(m_collection, &RoutineCollection::routineWasAdded, this, &RoutineCollectionModel::routineWasAdded);
    connect(m_collection, &RoutineCollection::routineWillBeRemoved, this, &RoutineCollectionModel::routineWillBeRemoved);
    connect(m_collection, &RoutineCollection::routineWasRemoved, this, &RoutineCollectionModel::routineWasRemoved);
}

void RoutineCollectionModel::routineWillBeAdded(photon::Routine *, int t_index)
{
    beginInsertRows(QModelIndex(), t_index, t_index);
}

void RoutineCollectionModel::routineWasAdded(photon::Routine *)
{
    endInsertRows();
}

void RoutineCollectionModel::routineWillBeRemoved(photon::Routine *, int t_index)
{
    beginRemoveRows(QModelIndex(), t_index, t_index);
}

void RoutineCollectionModel::routineWasRemoved(photon::Routine *)
{
    endRemoveRows();
}

int RoutineCollectionModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

 QModelIndex RoutineCollectionModel::index(int row, int column, const QModelIndex &parent) const
 {
     return createIndex(row, column, m_collection->routineAtIndex(row));
 }

 QModelIndex RoutineCollectionModel::parent(const QModelIndex &index) const
 {
     return QModelIndex();
 }

Qt::ItemFlags RoutineCollectionModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant RoutineCollectionModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
        default:
        return QVariant();

    case Qt::DisplayRole:
        return m_collection->routineAtIndex(index.row())->name();

    }
}

QVariant RoutineCollectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch(role)
    {
        default:
            return QVariant();

        case Qt::DisplayRole:
            return "Name";

    }
}

int RoutineCollectionModel::rowCount(const QModelIndex &) const
{
    return m_collection->routineCount();
}

RoutineCollectionPanel::RoutineCollectionPanel() : Panel("Routines"),m_impl(new Impl)
{
    QVBoxLayout *vLayout = new QVBoxLayout;


    m_impl->listView = new QListView;
    m_impl->listView->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));

    vLayout->addWidget(m_impl->listView);

    m_impl->addButton = new QPushButton("Add");
    m_impl->addButton->setEnabled(false);
    m_impl->removeButton = new QPushButton("Remove");
    m_impl->removeButton->setEnabled(false);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_impl->addButton);
    hLayout->addWidget(m_impl->removeButton);

    vLayout->addLayout(hLayout);


    setPanelLayout(vLayout);

    connect(m_impl->addButton, &QPushButton::clicked, this, &RoutineCollectionPanel::addClicked);
    connect(m_impl->removeButton, &QPushButton::clicked, this, &RoutineCollectionPanel::removeClicked);
    connect(m_impl->listView, &QListView::doubleClicked, this, &RoutineCollectionPanel::doubleClicked);
}

RoutineCollectionPanel::~RoutineCollectionPanel()
{
    delete m_impl;
}

void RoutineCollectionPanel::doubleClicked(const QModelIndex &t_index)
{
    Routine *routine = static_cast<Routine*>(t_index.internalPointer());

    if(routine)
    {
        photonApp->editRoutine(routine);
    }
}

void RoutineCollectionPanel::addClicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Routine Name",
                                         "Name", QLineEdit::Normal,
                                         "Untitled", &ok);
    if (ok && !text.isEmpty())
    {
        photonApp->project()->routines()->addRoutine(new Routine(text));
    }

}

void RoutineCollectionPanel::removeClicked()
{
    auto indicies = m_impl->listView->selectionModel()->selectedRows();

    QVector<Routine*> toDelete;
    for(auto &index : indicies)
    {
        toDelete.append(static_cast<Routine*>(index.internalPointer()));
    }

    for(auto seq : toDelete)
    {
        photonApp->project()->routines()->removeRoutine(seq);
    }

}

void RoutineCollectionPanel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    m_impl->removeButton->setEnabled(m_impl->listView->selectionModel()->hasSelection());
}

void RoutineCollectionPanel::projectDidOpen(photon::Project* project)
{
    RoutineCollectionModel *model = new RoutineCollectionModel(project->routines());
    m_impl->listView->setModel(model);

    m_impl->addButton->setEnabled(true);
    connect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RoutineCollectionPanel::selectionChanged);
}

void RoutineCollectionPanel::projectWillClose(photon::Project* project)
{
    disconnect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RoutineCollectionPanel::selectionChanged);
    m_impl->listView->setModel(nullptr);
    m_impl->addButton->setEnabled(false);
    m_impl->removeButton->setEnabled(false);
}


} // namespace photon
