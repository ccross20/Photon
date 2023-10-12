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

private slots:
    void addClicked();

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_WAVEFORMHEADER_H
