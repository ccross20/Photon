#ifndef RIGPANEL_P_H
#define RIGPANEL_P_H

#include <QTreeView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "rigpanel.h"
#include "scene/scenemodel.h"

namespace photon {

class RigPanel::Impl
{
public:
    QTreeView *treeView;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *duplicateButton;
    QVBoxLayout *vLayout;
    QHBoxLayout *hLayout;
    SceneModel *sceneModel;
    bool syncingFromProject = false;
};


}

#endif // RIGPANEL_P_H
