#ifndef PHOTON_FIXTUREGROUPEDITOR_H
#define PHOTON_FIXTUREGROUPEDITOR_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

// Properties-panel editor for a FixtureGroup: the shared name/tags fields plus
// the query (type / tags / zone) that decides which fixtures the group resolves
// to, and a live count of what currently matches.
class PHOTONCORE_EXPORT FixtureGroupEditor : public QWidget
{
    Q_OBJECT
public:
    explicit FixtureGroupEditor(FixtureGroup *group, QWidget *parent = nullptr);
    ~FixtureGroupEditor();

private slots:
    void queryEdited();
    void updateMatchCount();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTUREGROUPEDITOR_H
