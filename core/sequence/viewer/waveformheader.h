#ifndef PHOTON_WAVEFORMHEADER_H
#define PHOTON_WAVEFORMHEADER_H

#include <QWidget>

#include "photon-global.h"

namespace photon {

class WaveformHeader : public QWidget
{
    Q_OBJECT
public:
    explicit WaveformHeader(QWidget *parent = nullptr);
    ~WaveformHeader();

    void setSequence(Sequence *);

    void addAudioProcessor(AudioProcessor *);

signals:
    // Candidate markers for the waveform to preview; empty clears them.
    void previewMarkersChanged(const QVector<double> &times);
    // From the cue menu; handled by the waveform editor, which owns the
    // marker selection.
    void cutMarkersRequested();
    void copyMarkersRequested();
    void pasteMarkersRequested();

private slots:
    void addClicked();
    void deleteSelectedLayerClicked();
    void convertBeatsToMarkersClicked();

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_WAVEFORMHEADER_H
