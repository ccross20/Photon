#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include "tagcollectionpanel_p.h"
#include "photoncore.h"
#include "project/project.h"
#include "tag/tagcollection.h"

namespace photon {



TagCollectionModel::TagCollectionModel(TagCollection *t_collection) : QAbstractItemModel(),m_collection(t_collection)
{
    connect(m_collection, &TagCollection::tagWillBeAdded, this, &TagCollectionModel::tagWillBeAdded);
    connect(m_collection, &TagCollection::tagWasAdded, this, &TagCollectionModel::tagWasAdded);
    connect(m_collection, &TagCollection::tagWillBeRemoved, this, &TagCollectionModel::tagWillBeRemoved);
    connect(m_collection, &TagCollection::tagWasRemoved, this, &TagCollectionModel::tagWasRemoved);
}

void TagCollectionModel::tagWillBeAdded(QString, int t_index)
{
    beginInsertRows(QModelIndex(), t_index, t_index);
}

void TagCollectionModel::tagWasAdded(QString)
{
    endInsertRows();
}

void TagCollectionModel::tagWillBeRemoved(QString, int t_index)
{
    beginRemoveRows(QModelIndex(), t_index, t_index);
}

void TagCollectionModel::tagWasRemoved(QString)
{
    endRemoveRows();
}

int TagCollectionModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QModelIndex TagCollectionModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column, nullptr);
}

QModelIndex TagCollectionModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

Qt::ItemFlags TagCollectionModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant TagCollectionModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
    default:
        return QVariant();

    case Qt::DisplayRole:
        return m_collection->tagAtIndex(index.row()).toCaseFolded();

    }
}

QVariant TagCollectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch(role)
    {
    default:
        return QVariant();

    case Qt::DisplayRole:
        return "Name";

    }
}

int TagCollectionModel::rowCount(const QModelIndex &) const
{
    return m_collection->tagCount();
}

TagCollectionPanel::TagCollectionPanel() : Panel("photon.tag-collection"),m_impl(new Impl)
{
    QVBoxLayout *vLayout = new QVBoxLayout;

    setName("Tags");
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

    connect(m_impl->addButton, &QPushButton::clicked, this, &TagCollectionPanel::addClicked);
    connect(m_impl->removeButton, &QPushButton::clicked, this, &TagCollectionPanel::removeClicked);
    connect(m_impl->listView, &QListView::doubleClicked, this, &TagCollectionPanel::doubleClicked);
}

TagCollectionPanel::~TagCollectionPanel()
{
    delete m_impl;
}

void TagCollectionPanel::doubleClicked(const QModelIndex &t_index)
{
    /*
    Canvas *canvas = static_cast<Canvas*>(t_index.internalPointer());

    if(canvas)
    {
        CanvasDialog dialog(canvas);

        dialog.exec();
    }
*/
}

void TagCollectionPanel::addClicked()
{
    QInputDialog dialog;

    int result = dialog.exec();

    if (result == QInputDialog::DialogCode::Accepted)
    {
        photonApp->project()->tags()->addTag(dialog.textValue());
    }


}

void TagCollectionPanel::removeClicked()
{
    auto indicies = m_impl->listView->selectionModel()->selectedRows();

    QStringList toDelete;
    for(auto &index : indicies)
    {
        toDelete.append(index.data().toString());
    }

    for(const auto &seq : toDelete)
    {
        photonApp->project()->tags()->removeTag(seq);
    }

}

void TagCollectionPanel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    m_impl->removeButton->setEnabled(m_impl->listView->selectionModel()->hasSelection());
}

void TagCollectionPanel::projectDidOpen(photon::Project* project)
{
    TagCollectionModel *model = new TagCollectionModel(project->tags());
    m_impl->listView->setModel(model);

    m_impl->addButton->setEnabled(true);
    connect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TagCollectionPanel::selectionChanged);
}

void TagCollectionPanel::projectWillClose(photon::Project* project)
{
    disconnect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TagCollectionPanel::selectionChanged);
    m_impl->listView->setModel(nullptr);
    m_impl->addButton->setEnabled(false);
    m_impl->removeButton->setEnabled(false);
}



} // namespace photon
