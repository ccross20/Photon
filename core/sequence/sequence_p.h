#ifndef SEQUENCE_P_H
#define SEQUENCE_P_H

#include <atomic>
#include "sequence.h"
#include "cuelayer.h"
#include "audio/songdata.h"

namespace photon
{

class Sequence::Impl
{
public:
    Impl(Sequence *);
    void addLayer(Layer*);
    void removeLayer(Layer*);
    QVector<Layer*> layers;
    QVector<CueLayer*> cueLayers;
    QString name;
    QByteArray uniqueId;
    QString filePath;
    SongData songData;
    // Runtime-only: whether this sequence is owned by the Song Library (its
    // own .seq file, managed there) rather than embedded in the current
    // project's JSON. Never itself serialized - whoever creates/loads a
    // Sequence sets it once (Project leaves it false; the library flows set
    // it true).
    bool isLibrarySequence = false;
    Sequence *facade;

    // The editor's current playhead position, kept here (not on the QWidget-based
    // SequenceWidget) so that anything wanting to preview this sequence - notably
    // SequenceNode::evaluate(), which runs on keira's eval thread - can read it
    // without touching a widget. Atomic because it's written from the GUI thread
    // and read from the eval thread.
    std::atomic<double> previewTime{0.0};
};

}

#endif // SEQUENCE_P_H
