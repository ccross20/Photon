#include "surfacecollectionpanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include "surfacecollectionpanel_p.h"
#include "surface/surface.h"
#include "photoncore.h"
#include "project/project.h"
#include "surface/surfacecollection.h"

namespace photon {

SurfaceCollectionModel::SurfaceCollectionModel(SurfaceCollection *t_collection) : QAbstractItemModel(),m_collection(t_collection)
{
    connect(m_collection, &SurfaceCollection::surfaceWillBeAdded, this, &SurfaceCollectionModel::surfaceWillBeAdded);
    connect(m_collection, &SurfaceCollection::surfaceWasAdded, this, &SurfaceCollectionModel::surfaceWasAdded);
    connect(m_collection, &SurfaceCollection::surfaceWillBeRemoved, this, &SurfaceCollectionModel::surfaceWillBeRemoved);
    connect(m_collection, &SurfaceCollection::surfaceWasRemoved, this, &SurfaceCollectionModel::surfaceWasRemoved);
}

void SurfaceCollectionModel::surfaceWillBeAdded(photon::Surface *, int t_index)
{
    beginInsertRows(QModelIndex(), t_index, t_index);
}

void SurfaceCollectionModel::surfaceWasAdded(photon::Surface *, int t_index)
{
    endInsertRows();
}

void SurfaceCollectionModel::surfaceWillBeRemoved(photon::Surface *, int t_index)
{
    beginRemoveRows(QModelIndex(), t_index, t_index);
}

void SurfaceCollectionModel::surfaceWasRemoved(photon::Surface *, int t_index)
{
    endRemoveRows();
}

int SurfaceCollectionModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QModelIndex SurfaceCollectionModel::index(int row, int column, const QModelIndex &parent) const
{
    return createIndex(row, column, m_collection->surfaceAtIndex(row));
}

QModelIndex SurfaceCollectionModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

Qt::ItemFlags SurfaceCollectionModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant SurfaceCollectionModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
    default:
        return QVariant();

    case Qt::DisplayRole:
        return m_collection->surfaceAtIndex(index.row())->name();

    }
}

QVariant SurfaceCollectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch(role)
    {
    default:
        return QVariant();

    case Qt::DisplayRole:
        return "Name";

    }
}

int SurfaceCollectionModel::rowCount(const QModelIndex &) const
{
    return m_collection->surfaceCount();
}

SurfaceCollectionPanel::SurfaceCollectionPanel() : Panel("photon.surface-collection"),m_impl(new Impl)
{
    QVBoxLayout *vLayout = new QVBoxLayout;

    setName("Surfaces");
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

    connect(m_impl->addButton, &QPushButton::clicked, this, &SurfaceCollectionPanel::addClicked);
    connect(m_impl->removeButton, &QPushButton::clicked, this, &SurfaceCollectionPanel::removeClicked);
    connect(m_impl->listView, &QListView::doubleClicked, this, &SurfaceCollectionPanel::doubleClicked);
}

SurfaceCollectionPanel::~SurfaceCollectionPanel()
{
    delete m_impl;
}

void SurfaceCollectionPanel::doubleClicked(const QModelIndex &t_index)
{
    Surface *surface = static_cast<Surface*>(t_index.internalPointer());

    if(surface)
    {
        photonApp->surfaces()->editSurface(surface);
    }
}

void SurfaceCollectionPanel::addClicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Surface Name",
                                         "Name", QLineEdit::Normal,
                                         "Untitled", &ok);
    if (ok && !text.isEmpty())
    {
        Surface *surface = new Surface(text);
        surface->init();
        photonApp->surfaces()->addSurface(surface);
    }

}

void SurfaceCollectionPanel::removeClicked()
{
    auto indicies = m_impl->listView->selectionModel()->selectedRows();

    QVector<Surface*> toDelete;
    for(auto &index : indicies)
    {
        toDelete.append(static_cast<Surface*>(index.internalPointer()));
    }

    for(auto seq : toDelete)
    {
        photonApp->surfaces()->removeSurface(seq);
    }

}

void SurfaceCollectionPanel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    m_impl->removeButton->setEnabled(m_impl->listView->selectionModel()->hasSelection());
}

void SurfaceCollectionPanel::projectDidOpen(photon::Project* project)
{
    SurfaceCollectionModel *model = new SurfaceCollectionModel(photonApp->surfaces());
    m_impl->listView->setModel(model);
    m_impl->addButton->setEnabled(true);
    connect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SurfaceCollectionPanel::selectionChanged);
}

void SurfaceCollectionPanel::projectWillClose(photon::Project* project)
{
    disconnect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SurfaceCollectionPanel::selectionChanged);
    m_impl->listView->setModel(nullptr);
    m_impl->addButton->setEnabled(false);
    m_impl->removeButton->setEnabled(false);
}

} // namespace photon

