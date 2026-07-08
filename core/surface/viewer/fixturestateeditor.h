#ifndef FIXTURESTATEEDITOR_H
#define FIXTURESTATEEDITOR_H

#include <QWidget>
#include "photon-global.h"

class QVBoxLayout;

namespace photon {

// Custom node-editor widget for FixtureStateNode: manages the node's State by
// adding/removing capabilities and editing each capability's channel values.
class FixtureStateEditor : public QWidget
{
    Q_OBJECT
public:
    explicit FixtureStateEditor(State *state, QWidget *parent = nullptr);

public slots:
    void openAddMenu();

private:
    void rebuild();

    State *m_state = nullptr;
    QVBoxLayout *m_layout = nullptr;
};

} // namespace photon

#endif // FIXTURESTATEEDITOR_H
