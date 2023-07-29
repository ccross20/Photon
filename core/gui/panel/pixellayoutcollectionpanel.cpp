#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include "pixellayoutcollectionpanel_p.h"
#include "photoncore.h"
#include "project/project.h"
#include "pixel/pixellayout.h"
#include "pixel/pixellayoutcollection.h"
#include "gui/pixellayouteditor.h"

namespace photon {


PixelLayoutCollectionModel::PixelLayoutCollectionModel(PixelLayoutCollection *t_collection) : QAbstractItemModel(),m_collection(t_collection)
{
    connect(m_collection, &PixelLayoutCollection::layoutWillBeAdded, this, &PixelLayoutCollectionModel::layoutWillBeAdded);
    connect(m_collection, &PixelLayoutCollection::layoutWasAdded, this, &PixelLayoutCollectionModel::layoutWasAdded);
    connect(m_collection, &PixelLayoutCollection::layoutWillBeRemoved, this, &PixelLayoutCollectionModel::layoutWillBeRemoved);
    connect(m_collection, &PixelLayoutCollection::layoutWasRemoved, this, &PixelLayoutCollectionModel::layoutWasRemoved);
}

void PixelLayoutCollectionModel::layoutWillBeAdded(photon::PixelLayout *, int t_index)
{
    beginInsertRows(QModelIndex(), t_index, t_index);
}

void PixelLayoutCollectionModel::layoutWasAdded(photon::PixelLayout *)
{
    endInsertRows();
}

void PixelLayoutCollectionModel::layoutWillBeRemoved(photon::PixelLayout *, int t_index)
{
    beginRemoveRows(QModelIndex(), t_index, t_index);
}

void PixelLayoutCollectionModel::layoutWasRemoved(photon::PixelLayout *)
{
    endRemoveRows();
}

int PixelLayoutCollectionModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

 QModelIndex PixelLayoutCollectionModel::index(int row, int column, const QModelIndex &parent) const
 {
     return createIndex(row, column, m_collection->layoutAtIndex(row));
 }

 QModelIndex PixelLayoutCollectionModel::parent(const QModelIndex &index) const
 {
     return QModelIndex();
 }

Qt::ItemFlags PixelLayoutCollectionModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant PixelLayoutCollectionModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
        default:
        return QVariant();

    case Qt::DisplayRole:
        return m_collection->layoutAtIndex(index.row())->name();

    }
}

QVariant PixelLayoutCollectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch(role)
    {
        default:
            return QVariant();

        case Qt::DisplayRole:
            return "Name";

    }
}

int PixelLayoutCollectionModel::rowCount(const QModelIndex &) const
{
    return m_collection->layoutCount();
}

PixelLayoutCollectionPanel::PixelLayoutCollectionPanel() : Panel("photon.pixellayout-collection"),m_impl(new Impl)
{
    QVBoxLayout *vLayout = new QVBoxLayout;

    setName("Pixel Layouts");
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

    connect(m_impl->addButton, &QPushButton::clicked, this, &PixelLayoutCollectionPanel::addClicked);
    connect(m_impl->removeButton, &QPushButton::clicked, this, &PixelLayoutCollectionPanel::removeClicked);
    connect(m_impl->listView, &QListView::doubleClicked, this, &PixelLayoutCollectionPanel::doubleClicked);
}

PixelLayoutCollectionPanel::~PixelLayoutCollectionPanel()
{
    delete m_impl;
}

void PixelLayoutCollectionPanel::doubleClicked(const QModelIndex &t_index)
{
    PixelLayout *layout = static_cast<PixelLayout*>(t_index.internalPointer());

    if(layout)
    {
        PixelLayoutEditor *editor = new PixelLayoutEditor(layout);

        editor->show();
    }
}

void PixelLayoutCollectionPanel::addClicked()
{
    PixelLayout *layout = new PixelLayout;
    layout->setName("Test");
    photonApp->project()->pixelLayouts()->addLayout(layout);


    PixelLayoutEditor *editor = new PixelLayoutEditor(layout);

    editor->show();
    /*
    CanvasDialog dialog;

    int result = dialog.exec();

    if (result == CanvasDialog::DialogCode::Accepted)
    {
        photonApp->project()->pixelLayouts()->addLayout(dialog.canvas());
    }
*/

}

void PixelLayoutCollectionPanel::removeClicked()
{
    auto indicies = m_impl->listView->selectionModel()->selectedRows();

    QVector<PixelLayout*> toDelete;
    for(auto &index : indicies)
    {
        toDelete.append(static_cast<PixelLayout*>(index.internalPointer()));
    }

    for(auto seq : toDelete)
    {
        photonApp->project()->pixelLayouts()->removeLayout(seq);
    }

}

void PixelLayoutCollectionPanel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    m_impl->removeButton->setEnabled(m_impl->listView->selectionModel()->hasSelection());
}

void PixelLayoutCollectionPanel::projectDidOpen(photon::Project* project)
{
    PixelLayoutCollectionModel *model = new PixelLayoutCollectionModel(project->pixelLayouts());
    m_impl->listView->setModel(model);

    m_impl->addButton->setEnabled(true);
    connect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PixelLayoutCollectionPanel::selectionChanged);
}

void PixelLayoutCollectionPanel::projectWillClose(photon::Project* project)
{
    disconnect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PixelLayoutCollectionPanel::selectionChanged);
    m_impl->listView->setModel(nullptr);
    m_impl->addButton->setEnabled(false);
    m_impl->removeButton->setEnabled(false);
}


} // namespace photon
