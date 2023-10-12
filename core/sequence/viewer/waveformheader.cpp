#include <QPainter>
#include <QVBoxLayout>
#include <QMenu>
#include "waveformheader_p.h"
#include "sequence/sequence.h"
#include "plugin/pluginfactory.h"
#include "audio/audioprocessor.h"
#include "gui/menufactory.h"
#include "photoncore.h"
#include "sequence/beatlayer.h"

namespace photon {

BeatLayerDelegate::BeatLayerDelegate(QObject *parent) : QAbstractItemDelegate(parent)
{

}

void BeatLayerDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect iconRect = QRect(option.rect.topLeft(),option.rect.size()).adjusted(2,2,-2,-2);


    painter->fillRect(iconRect, index.data().toBool() ? Qt::green : Qt::red);
}

bool BeatLayerDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    bool curValue = index.data().toBool();
    if(event->type() == QEvent::MouseButtonPress)
    {
        model->setData(index,!curValue);
        return true;
    }

    return false;
}

QSize BeatLayerDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
        return QSize(32,32);
}



void BeatLayerModel::beatLayerAdded(photon::BeatLayer *t_layer)
{
    beginResetModel();
    endResetModel();
}

void BeatLayerModel::beatLayerRemoved(photon::BeatLayer *t_layer)
{
    beginResetModel();
    endResetModel();
}

QVariant BeatLayerModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || !m_sequence)
        return QVariant();
    auto beat = m_sequence->beatLayers()[index.row()];

    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (index.column()) {
            case 0:
                return beat->name();
            case 1:
                return beat->isSnappable();
            case 2:
                return beat->isVisible();
            case 3:
                return beat->isEditable();
        }

    }

    return QVariant();
}

QVariant BeatLayerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

QModelIndex BeatLayerModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!m_sequence)
        return QModelIndex();
    return createIndex(row, column, m_sequence->beatLayers()[row]);
}

QModelIndex BeatLayerModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int BeatLayerModel::rowCount(const QModelIndex &parent) const
{
    if(!m_sequence || parent.isValid())
        return 0;
    return m_sequence->beatLayers().length();
}

int BeatLayerModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}


Qt::ItemFlags BeatLayerModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool BeatLayerModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid() || !m_sequence)
        return false;
    auto beat = m_sequence->beatLayers()[index.row()];

    if(role == Qt::EditRole)
    {
        switch (index.column()) {
            case 0:
                beat->setName(value.toString());
            break;
            case 1:
                beat->setIsSnappable(value.toBool());
            break;
            case 2:
                beat->setIsVisible(value.toBool());
            break;
            case 3:
                m_sequence->setEditableBeatLayer(beat);
                //beat->setIsEditable(value.toBool());
            break;
        }
        return true;
    }

    return false;
}

void BeatLayerModel::setSequence(Sequence *t_sequence)
{
    beginResetModel();
    m_sequence = t_sequence;

    connect(m_sequence, &Sequence::beatLayerAdded, this, &BeatLayerModel::beatLayerAdded);
    connect(m_sequence, &Sequence::beatLayerRemoved, this, &BeatLayerModel::beatLayerRemoved);
    connect(m_sequence, &Sequence::editableBeatLayerChanged, this, &BeatLayerModel::beatLayerRemoved);

    endResetModel();
}




WaveformHeader::WaveformHeader(QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    m_impl->delegate = new BeatLayerDelegate;
    m_impl->beatModel = new BeatLayerModel;
    m_impl->tree = new QTreeView;
    m_impl->tree->setHeaderHidden(true);
    m_impl->tree->setModel(m_impl->beatModel);
    m_impl->tree->setItemDelegateForColumn(1, m_impl->delegate);
    m_impl->tree->setItemDelegateForColumn(2, m_impl->delegate);
    m_impl->tree->setItemDelegateForColumn(3, m_impl->delegate);
    m_impl->tree->setColumnWidth(1,32);
    m_impl->tree->setColumnWidth(2,32);
    m_impl->tree->setColumnWidth(3,32);

    auto vLayout = new QVBoxLayout;

    m_impl->addButton = new QPushButton("Add");
    vLayout->addWidget(m_impl->tree);
    vLayout->addWidget(m_impl->addButton);
    vLayout->addStretch();
    setLayout(vLayout);

    connect(m_impl->addButton, &QPushButton::clicked, this, &WaveformHeader::addClicked);
}

WaveformHeader::~WaveformHeader()
{
    delete m_impl;
}

void WaveformHeader::addClicked()
{

    QMenu itemMenu;
    itemMenu.addAction("Add Layer",[this](){
        BeatLayer *layer = new BeatLayer;
        layer->setName("New Layer");
        m_impl->sequence->addBeatLayer(layer);
        m_impl->sequence->setEditableBeatLayer(layer);
    });



    MenuFactory<AudioProcessorInformation> factory;
    auto effects = photonApp->plugins()->audioProcessors();
    for(auto &info : effects)
    {
        factory.addItem(info.categories, info);
    }
    auto processorMenu = factory.createMenu();
    itemMenu.addMenu(processorMenu);

    connect(processorMenu, &QMenu::triggered, this, [this](QAction *action){
        addAudioProcessor(photonApp->plugins()->createAudioProcessor(action->data().toByteArray()));
    });

    itemMenu.exec(m_impl->addButton->mapToGlobal(QPoint{}));

}

void WaveformHeader::addAudioProcessor(AudioProcessor *t_processor)
{
    t_processor->init(m_impl->sequence);
    t_processor->startProcessing();

    connect(t_processor, &AudioProcessor::completed,this,[t_processor](){
        delete t_processor;
    });
}

void WaveformHeader::setSequence(Sequence *t_sequence)
{
    m_impl->sequence = t_sequence;
    m_impl->beatModel->setSequence(t_sequence);
}

} // namespace photon
