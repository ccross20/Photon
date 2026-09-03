#ifndef FIXTURESTATEEDITOR_H
#define FIXTURESTATEEDITOR_H

#include <QWidget>
#include "photon-global.h"

class QVBoxLayout;
class QScrollArea;

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
    QScrollArea *m_scroll = nullptr;
    // Layout that holds the per-capability frames; lives inside m_scroll's
    // content widget so a long list scrolls instead of squashing.
    QVBoxLayout *m_listLayout = nullptr;
};

} // namespace photon

#endif // FIXTURESTATEEDITOR_H
