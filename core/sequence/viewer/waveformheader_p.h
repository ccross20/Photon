#ifndef WAVEFORMHEADER_P_H
#define WAVEFORMHEADER_P_H

#include <QAbstractItemDelegate>
#include <QPushButton>
#include <QTreeView>
#include <QAbstractItemModel>
#include "waveformheader.h"

namespace photon {

class CueLayerDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    CueLayerDelegate(QObject *parent = Q_NULLPTR);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

class CueLayerModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    void setSequence(Sequence *);

private slots:
    void cueLayerAdded(photon::CueLayer *);
    void cueLayerRemoved(photon::CueLayer *);

private:
    Sequence *m_sequence = nullptr;
};





class WaveformHeader::Impl
{
public:
    QTreeView *tree = nullptr;
    CueLayerModel *cueModel = nullptr;
    CueLayerDelegate *delegate;
    Sequence *sequence;
    QPushButton *addButton;
    QPushButton *layerMenuButton;
};

}


#endif // WAVEFORMHEADER_P_H
