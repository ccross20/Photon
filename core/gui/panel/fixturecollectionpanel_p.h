#ifndef FIXTURECOLLECTIONPANEL_P_H
#define FIXTURECOLLECTIONPANEL_P_H

#include <QTreeView>
#include <QVBoxLayout>
#include <QPushButton>
#include "fixturecollectionpanel.h"
#include "scene/scenemodel.h"

namespace photon {

class FixtureCollectionPanel::Impl
{
public:
    QTreeView *treeView;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *duplicateButton;
    QWidget *editorWidget;
    QVBoxLayout *vLayout;
    SceneModel *sceneModel;
};


}

#endif // FIXTURECOLLECTIONPANEL_P_H
