
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include "sequencecollectionpanel_p.h"
#include "sequence/sequence.h"
#include "photoncore.h"
#include "project/project.h"
#include "sequence/sequencecollection.h"

namespace photon {

SequenceCollectionModel::SequenceCollectionModel(SequenceCollection *t_collection) : QAbstractItemModel(),m_collection(t_collection)
{
    connect(m_collection, &SequenceCollection::sequenceWillBeAdded, this, &SequenceCollectionModel::sequenceWillBeAdded);
    connect(m_collection, &SequenceCollection::sequenceWasAdded, this, &SequenceCollectionModel::sequenceWasAdded);
    connect(m_collection, &SequenceCollection::sequenceWillBeRemoved, this, &SequenceCollectionModel::sequenceWillBeRemoved);
    connect(m_collection, &SequenceCollection::sequenceWasRemoved, this, &SequenceCollectionModel::sequenceWasRemoved);
}

void SequenceCollectionModel::sequenceWillBeAdded(photon::Sequence *, int t_index)
{
    beginInsertRows(QModelIndex(), t_index, t_index);
}

void SequenceCollectionModel::sequenceWasAdded(photon::Sequence *, int t_index)
{
    endInsertRows();
}

void SequenceCollectionModel::sequenceWillBeRemoved(photon::Sequence *, int t_index)
{
    beginRemoveRows(QModelIndex(), t_index, t_index);
}

void SequenceCollectionModel::sequenceWasRemoved(photon::Sequence *, int t_index)
{
    endRemoveRows();
}

int SequenceCollectionModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

 QModelIndex SequenceCollectionModel::index(int row, int column, const QModelIndex &parent) const
 {
     return createIndex(row, column, m_collection->sequenceAtIndex(row));
 }

 QModelIndex SequenceCollectionModel::parent(const QModelIndex &index) const
 {
     return QModelIndex();
 }

Qt::ItemFlags SequenceCollectionModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant SequenceCollectionModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
        default:
        return QVariant();

    case Qt::DisplayRole:
        return m_collection->sequenceAtIndex(index.row())->name();

    }
}

QVariant SequenceCollectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch(role)
    {
        default:
            return QVariant();

        case Qt::DisplayRole:
            return "Name";

    }
}

int SequenceCollectionModel::rowCount(const QModelIndex &) const
{
    return m_collection->sequenceCount();
}

SequenceCollectionPanel::SequenceCollectionPanel() : Panel("Sequences"),m_impl(new Impl)
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

    connect(m_impl->addButton, &QPushButton::clicked, this, &SequenceCollectionPanel::addClicked);
    connect(m_impl->removeButton, &QPushButton::clicked, this, &SequenceCollectionPanel::removeClicked);
    connect(m_impl->listView, &QListView::doubleClicked, this, &SequenceCollectionPanel::doubleClicked);
}

SequenceCollectionPanel::~SequenceCollectionPanel()
{
    delete m_impl;
}

void SequenceCollectionPanel::doubleClicked(const QModelIndex &t_index)
{
    Sequence *sequence = static_cast<Sequence*>(t_index.internalPointer());

    if(sequence)
    {
        photonApp->editSequence(sequence);
    }
}

void SequenceCollectionPanel::addClicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Sequence Name",
                                         "Name", QLineEdit::Normal,
                                         "Untitled", &ok);
    if (ok && !text.isEmpty())
    {
        Sequence *sequence = new Sequence(text);
        sequence->init();
        photonApp->project()->addSequence(sequence);
    }

}

void SequenceCollectionPanel::removeClicked()
{
    auto indicies = m_impl->listView->selectionModel()->selectedRows();

    QVector<Sequence*> toDelete;
    for(auto &index : indicies)
    {
        toDelete.append(static_cast<Sequence*>(index.internalPointer()));
    }

    for(auto seq : toDelete)
    {
        photonApp->project()->sequences()->removeSequence(seq);
    }

}

void SequenceCollectionPanel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    m_impl->removeButton->setEnabled(m_impl->listView->selectionModel()->hasSelection());
}

void SequenceCollectionPanel::projectDidOpen(photon::Project* project)
{
    SequenceCollectionModel *model = new SequenceCollectionModel(project->sequences());
    m_impl->listView->setModel(model);
    m_impl->addButton->setEnabled(true);
    connect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SequenceCollectionPanel::selectionChanged);
}

void SequenceCollectionPanel::projectWillClose(photon::Project* project)
{
    disconnect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SequenceCollectionPanel::selectionChanged);
    m_impl->listView->setModel(nullptr);
    m_impl->addButton->setEnabled(false);
    m_impl->removeButton->setEnabled(false);
}

} // namespace photon
