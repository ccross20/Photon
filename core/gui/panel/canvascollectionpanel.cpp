#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include "canvascollectionpanel_p.h"
#include "pixel/canvas.h"
#include "photoncore.h"
#include "project/project.h"
#include "pixel/canvascollection.h"
#include "gui/dialog/canvasdialog.h"



namespace photon {


CanvasCollectionModel::CanvasCollectionModel(CanvasCollection *t_collection) : QAbstractItemModel(),m_collection(t_collection)
{
    connect(m_collection, &CanvasCollection::canvasWillBeAdded, this, &CanvasCollectionModel::canvasWillBeAdded);
    connect(m_collection, &CanvasCollection::canvasWasAdded, this, &CanvasCollectionModel::canvasWasAdded);
    connect(m_collection, &CanvasCollection::canvasWillBeRemoved, this, &CanvasCollectionModel::canvasWillBeRemoved);
    connect(m_collection, &CanvasCollection::canvasWasRemoved, this, &CanvasCollectionModel::canvasWasRemoved);
}

void CanvasCollectionModel::canvasWillBeAdded(photon::Canvas *, int t_index)
{
    beginInsertRows(QModelIndex(), t_index, t_index);
}

void CanvasCollectionModel::canvasWasAdded(photon::Canvas *)
{
    endInsertRows();
}

void CanvasCollectionModel::canvasWillBeRemoved(photon::Canvas *, int t_index)
{
    beginRemoveRows(QModelIndex(), t_index, t_index);
}

void CanvasCollectionModel::canvasWasRemoved(photon::Canvas *)
{
    endRemoveRows();
}

int CanvasCollectionModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

 QModelIndex CanvasCollectionModel::index(int row, int column, const QModelIndex &parent) const
 {
     return createIndex(row, column, m_collection->canvasAtIndex(row));
 }

 QModelIndex CanvasCollectionModel::parent(const QModelIndex &index) const
 {
     return QModelIndex();
 }

Qt::ItemFlags CanvasCollectionModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant CanvasCollectionModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
        default:
        return QVariant();

    case Qt::DisplayRole:
        return m_collection->canvasAtIndex(index.row())->name();

    }
}

QVariant CanvasCollectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch(role)
    {
        default:
            return QVariant();

        case Qt::DisplayRole:
            return "Name";

    }
}

int CanvasCollectionModel::rowCount(const QModelIndex &) const
{
    return m_collection->canvasCount();
}

CanvasCollectionPanel::CanvasCollectionPanel() : Panel("photon.canvas-collection"),m_impl(new Impl)
{
    QVBoxLayout *vLayout = new QVBoxLayout;

    setName("Canvas");
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

    connect(m_impl->addButton, &QPushButton::clicked, this, &CanvasCollectionPanel::addClicked);
    connect(m_impl->removeButton, &QPushButton::clicked, this, &CanvasCollectionPanel::removeClicked);
    connect(m_impl->listView, &QListView::doubleClicked, this, &CanvasCollectionPanel::doubleClicked);
}

CanvasCollectionPanel::~CanvasCollectionPanel()
{
    delete m_impl;
}

void CanvasCollectionPanel::doubleClicked(const QModelIndex &t_index)
{
    Canvas *canvas = static_cast<Canvas*>(t_index.internalPointer());

    if(canvas)
    {
        CanvasDialog dialog(canvas);

        dialog.exec();
    }
}

void CanvasCollectionPanel::addClicked()
{
    CanvasDialog dialog;

    int result = dialog.exec();

    if (result == CanvasDialog::DialogCode::Accepted)
    {
        photonApp->project()->canvases()->addCanvas(dialog.canvas());
    }


}

void CanvasCollectionPanel::removeClicked()
{
    auto indicies = m_impl->listView->selectionModel()->selectedRows();

    QVector<Canvas*> toDelete;
    for(auto &index : indicies)
    {
        toDelete.append(static_cast<Canvas*>(index.internalPointer()));
    }

    for(auto seq : toDelete)
    {
        photonApp->project()->canvases()->removeCanvas(seq);
    }

}

void CanvasCollectionPanel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    m_impl->removeButton->setEnabled(m_impl->listView->selectionModel()->hasSelection());
}

void CanvasCollectionPanel::projectDidOpen(photon::Project* project)
{
    CanvasCollectionModel *model = new CanvasCollectionModel(project->canvases());
    m_impl->listView->setModel(model);

    m_impl->addButton->setEnabled(true);
    connect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CanvasCollectionPanel::selectionChanged);
}

void CanvasCollectionPanel::projectWillClose(photon::Project* project)
{
    disconnect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CanvasCollectionPanel::selectionChanged);
    m_impl->listView->setModel(nullptr);
    m_impl->addButton->setEnabled(false);
    m_impl->removeButton->setEnabled(false);
}


} // namespace photon
