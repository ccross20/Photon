#ifndef PHOTON_SEQUENCECOLLECTION_H
#define PHOTON_SEQUENCECOLLECTION_H

#include <QObject>
#include "photon-global.h"

namespace photon {
class SequencePanel;

class PHOTONCORE_EXPORT SequenceCollection : public QObject
{
    Q_OBJECT
public:
    // ownsSequences: whether clear()/the destructor delete the Sequences
    // they hold. True for PhotonCore's app-wide collection, the actual owner
    // of every Sequence's lifetime; false for Project's own collection
    // (see Project::sequences()), which is just a membership list over the
    // same objects and must not double-delete them.
    explicit SequenceCollection(bool ownsSequences = true, QObject *parent = nullptr);
    ~SequenceCollection();

    void clear();
    int sequenceCount() const;
    Sequence *sequenceAtIndex(uint) const;
    const QVector<Sequence*> &sequences() const;

    void editSequence(Sequence *);
    void setActiveSequencePanel(SequencePanel *panel);
    SequencePanel *activeSequencePanel() const;
    Sequence *activeSequence() const;
    // The sequence's open editor panel, or nullptr if its tab has been
    // closed (it stays loaded here either way - see panelDestroyed()).
    SequencePanel *panelFor(Sequence *) const;

signals:
    void sequenceWillBeAdded(photon::Sequence *, int);
    void sequenceWasAdded(photon::Sequence *, int);
    void sequenceWillBeRemoved(photon::Sequence *, int);
    void sequenceWasRemoved(photon::Sequence *, int);

public slots:
    void addSequence(photon::Sequence *);
    void removeSequence(photon::Sequence *);
    void panelDestroyed(QObject*);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SEQUENCECOLLECTION_H
