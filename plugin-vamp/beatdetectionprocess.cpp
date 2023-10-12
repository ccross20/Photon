#include "beatdetectionprocess.h"
#include "sequence/sequence.h"
#include "sequence/beatlayer.h"

namespace photon {

BeatDetectionProcess::BeatDetectionProcess()
{
    m_vamp.setPluginName("vamp-example-plugins");
    m_vamp.setPluginId("percussiononsets");

    connect(&m_vamp, &Vamp::processingComplete, this, &BeatDetectionProcess::complete);
}

void BeatDetectionProcess::startProcessing()
{

    m_vamp.setFile(audioFilePath());
    m_vamp.start();
}

void BeatDetectionProcess::processingComplete()
{
    BeatLayer *layer = new BeatLayer;
    layer->setName("Beats");
    layer->addBeats(m_vamp.beats());
    sequence()->addBeatLayer(layer);

}

AudioProcessorInformation BeatDetectionProcess::info()
{
    AudioProcessorInformation toReturn([](){return new BeatDetectionProcess;});
    toReturn.name = "Beats";
    toReturn.id = "photon.audio-process.beat";

    return toReturn;
}

} // namespace photon
