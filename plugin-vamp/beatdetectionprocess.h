#ifndef PHOTON_BEATDETECTIONPROCESS_H
#define PHOTON_BEATDETECTIONPROCESS_H

#include "audio/audioprocessor.h"
#include "vamp.h"

namespace photon {

class BeatDetectionProcess : public AudioProcessor
{
public:
    BeatDetectionProcess();

    void startProcessing() override;
    void processingComplete() override;

    static AudioProcessorInformation info();

private:

    Vamp m_vamp;
};

} // namespace photon

#endif // PHOTON_BEATDETECTIONPROCESS_H
