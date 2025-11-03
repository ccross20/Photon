#ifndef ACTIONMODEL_H
#define ACTIONMODEL_H

#include <QObject>
#include "data/commonmodel.h"

namespace photon {

class ActionFalloffData;
class ActionEffectFolderData;
class ActionParameterData;
class ActionMaskData;
class ActionStateData;
class FixtureAction;

class ActionData : public AbstractTreeData
{
    Q_OBJECT
public:
    ActionData(SurfaceAction *);
    SurfaceAction *action() const{return m_action;}

private slots:
    void channelAdded(photon::Channel *);
    void channelRemoved(photon::Channel *);
    void channelMoved(photon::Channel *);

private:
    ActionMaskData *m_maskFolder;
    ActionFalloffData *m_falloffData;
    FolderData *m_channelFolder;
    ActionStateData *m_stateData;
    ActionParameterData *m_parameterData;
    ActionEffectFolderData *m_actionEffectData;
    SurfaceAction *m_action;

};


class ActionFalloffData : public AbstractTreeData
{
    Q_OBJECT
public:
    ActionFalloffData(FixtureAction*);
    FalloffEffectData *findEffectData(FalloffEffect *);

    FixtureAction *action() const{return m_action;}

private slots:
    void effectAdded(photon::FalloffEffect*);
    void effectRemoved(photon::FalloffEffect*);
    void effectMoved(photon::FalloffEffect*);

private:
    FixtureAction *m_action;
};


class ActionEffectFolderData : public AbstractTreeData
{
    Q_OBJECT
public:
    ActionEffectFolderData(SurfaceAction*);
    BaseEffectData *findEffectData(BaseEffect *);

    SurfaceAction *action() const{return m_action;}

private slots:
    void effectAdded(photon::BaseEffect*);
    void effectRemoved(photon::BaseEffect*);
    void effectMoved(photon::BaseEffect*);

private:
    SurfaceAction *m_action;
};

class ActionParameterData : public AbstractTreeData
{
    Q_OBJECT
public:
    ActionParameterData(SurfaceAction*);

    SurfaceAction *action() const{return m_action;}

private:
    SurfaceAction *m_action;
};


class ActionMaskData : public AbstractTreeData
{
    Q_OBJECT
public:
    ActionMaskData(FixtureAction*);
    MaskEffectData *findEffectData(MaskEffect *);

    FixtureAction *action() const{return m_action;}

private slots:
    void effectAdded(photon::MaskEffect*);
    void effectRemoved(photon::MaskEffect*);
    void effectMoved(photon::MaskEffect*);

private:
    FixtureAction *m_action;
};




class ActionStateData : public AbstractTreeData
{
public:
    ActionStateData(FixtureAction*);

    State *state() const;
    FixtureAction *action() const{return m_action;}

private:
    FixtureAction *m_action;
};

class ActionModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ActionModel();
    ~ActionModel();

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

    void addAction(SurfaceAction *);
    void removeAction(SurfaceAction *);
    void setContainer(SurfaceGizmoContainer *);
    QVector<SurfaceAction*> actions() const;
    RootData *root() const{return m_root;}

private slots:
    void childWillBeAdded(photon::AbstractTreeData*, int);
    void childWasAdded(photon::AbstractTreeData*);
    void childWillBeRemoved(photon::AbstractTreeData*, int);
    void childWasRemoved(photon::AbstractTreeData*);
    void metadataUpdated(photon::AbstractTreeData*);

    void actionWasAdded(photon::SurfaceAction *);
    void actionWasRemoved(photon::SurfaceAction *);


private:
    RootData *m_root;
    SurfaceGizmoContainer *m_container = nullptr;
};

} // namespace photon

#endif // ACTIONMODEL_H
