#ifndef FIXTURESTATEEDITOR_H
#define FIXTURESTATEEDITOR_H

#include <QWidget>
#include "photon-global.h"

class QVBoxLayout;

namespace photon {

class FixtureStateNode;

// Custom node-editor widget for FixtureStateNode: manages the node's State by
// adding/removing capabilities, editing each capability's channel values, and
// exposing individual channels as graph input ports.
class FixtureStateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit FixtureStateEditor(FixtureStateNode *node, QWidget *parent = nullptr);

public slots:
    void openAddMenu();

private:
    void rebuild();

    FixtureStateNode *m_node = nullptr;
    QVBoxLayout *m_layout = nullptr;
};

} // namespace photon

#endif // FIXTURESTATEEDITOR_H
