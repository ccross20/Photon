#ifndef SEQUENCE_P_H
#define SEQUENCE_P_H

#include <atomic>
#include "sequence.h"
#include "beatlayer.h"
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
    QVector<BeatLayer*> beatLayers;
    QString name;
    QString filePath;
    SongData songData;
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
