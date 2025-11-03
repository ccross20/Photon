#ifndef PHOTON_CLIPMODEL_H
#define PHOTON_CLIPMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "photon-global.h"
#include "data/commonmodel.h"

namespace photon {

class ClipFalloffData;
class BaseEffectData;
class ClipEffectFolderData;
class ClipParameterData;
class PixelLayoutFolderData;
class ClipMaskData;
class FixtureClip;
class ClipStateData;


class ClipData : public AbstractTreeData
{
    Q_OBJECT
public:
    ClipData(Clip *);
    Clip *clip() const{return m_clip;}

private slots:
    void channelAdded(photon::Channel *);
    void channelRemoved(photon::Channel *);
    void channelMoved(photon::Channel *);

private:
    ClipMaskData *m_maskFolder;
    ClipFalloffData *m_falloffData;
    FolderData *m_channelFolder;
    ClipStateData *m_stateData;
    ClipParameterData *m_parameterData;
    PixelLayoutFolderData *m_pixelLayoutData;
    ClipEffectFolderData *m_clipEffectData;
    Clip *m_clip;

};

class MasterLayerData : public AbstractTreeData
{
    Q_OBJECT
public:
    MasterLayerData(MasterLayer *);
    MasterLayer *layer() const{return m_layer;}

private:
    FolderData *m_channelFolder;
    MasterLayer *m_layer;

};

class ClipFalloffData : public AbstractTreeData
{
    Q_OBJECT
public:
    ClipFalloffData(FixtureClip*);
    FalloffEffectData *findEffectData(FalloffEffect *);

    FixtureClip *clip() const{return m_clip;}

private slots:
    void effectAdded(photon::FalloffEffect*);
    void effectRemoved(photon::FalloffEffect*);
    void effectMoved(photon::FalloffEffect*);

private:
    FixtureClip *m_clip;
};

class PixelLayoutData : public AbstractTreeData
{
public:
    PixelLayoutData(PixelLayout*);
    PixelLayout *pixelLayout() const{return m_pixelLayout;}

private:
    PixelLayout *m_pixelLayout;

};

class PixelLayoutFolderData : public AbstractTreeData
{
    Q_OBJECT
public:
    PixelLayoutFolderData(CanvasLayerGroup*);
    PixelLayoutData *findPixelLayoutData(PixelLayout *);

    CanvasLayerGroup *layer() const{return m_layer;}

private slots:
    void pixelLayoutAdded(photon::PixelLayout*);
    void pixelLayoutRemoved(photon::PixelLayout*);

private:
    CanvasLayerGroup *m_layer;
};

class ClipEffectFolderData : public AbstractTreeData
{
    Q_OBJECT
public:
    ClipEffectFolderData(Clip*);
    BaseEffectData *findEffectData(BaseEffect *);

    Clip *clip() const{return m_clip;}

private slots:
    void effectAdded(photon::BaseEffect*);
    void effectRemoved(photon::BaseEffect*);
    void effectMoved(photon::BaseEffect*);

private:
    Clip *m_clip;
};

class ClipParameterData : public AbstractTreeData
{
    Q_OBJECT
public:
    ClipParameterData(Clip*);

    Clip *clip() const{return m_clip;}

private:
    Clip *m_clip;
};


class ClipMaskData : public AbstractTreeData
{
    Q_OBJECT
public:
    ClipMaskData(FixtureClip*);
    MaskEffectData *findEffectData(MaskEffect *);

    FixtureClip *clip() const{return m_clip;}

private slots:
    void effectAdded(photon::MaskEffect*);
    void effectRemoved(photon::MaskEffect*);
    void effectMoved(photon::MaskEffect*);

private:
    FixtureClip *m_clip;
};




class ClipStateData : public AbstractTreeData
{
public:
    ClipStateData(FixtureClip*);

    State *state() const;
    FixtureClip *clip() const{return m_clip;}

private:
    FixtureClip *m_clip;
};

class ClipModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ClipModel();
    ~ClipModel();

    AbstractTreeData *dataForIndex(const QModelIndex &) const;
    QModelIndex indexForData(AbstractTreeData *) const;
    QModelIndex indexForId(const QByteArray &) const;

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

    void addClip(Clip *);
    void removeClip(Clip *);
    QVector<Clip*> clips() const;
    void addMasterLayer(MasterLayer *);
    void removeMasterLayer(MasterLayer *);
    RootData *root() const{return m_root;}

private slots:
    void childWillBeAdded(photon::AbstractTreeData*, int);
    void childWasAdded(photon::AbstractTreeData*);
    void childWillBeRemoved(photon::AbstractTreeData*, int);
    void childWasRemoved(photon::AbstractTreeData*);
    void metadataUpdated(photon::AbstractTreeData*);

private:
    RootData *m_root;
};

} // namespace photon

#endif // PHOTON_CLIPMODEL_H
