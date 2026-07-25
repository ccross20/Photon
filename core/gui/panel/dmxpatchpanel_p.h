#ifndef DMXPATCHPANEL_P_H
#define DMXPATCHPANEL_P_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QScrollArea>
#include "dmxpatchpanel.h"
#include "gui/dmxpatch/dmxpatchgrid.h"
#include "graph/bus/identifyfixturenode.h"

namespace photon {

class DMXPatchPanel::Impl
{
public:
    QVBoxLayout *layout;
    QHBoxLayout *toolbarLayout;
    QLabel *universeLabel;
    QSpinBox *universeSpin;
    QPushButton *moveToUniverseButton;
    QPushButton *identifyButton;
    QScrollArea *scrollArea;
    DMXPatchGrid *grid;

    // The bus graph's fixed "Identify" node, resolved per-project since the
    // bus graph is recreated on every project load.
    IdentifyFixtureNode *identifyNode = nullptr;
};

}

#endif // DMXPATCHPANEL_P_H
