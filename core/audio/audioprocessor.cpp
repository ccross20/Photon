#include "audioprocessor.h"
#include "sequence/sequence.h"

namespace photon {

class AudioProcessor::Impl
{
public:
    Sequence *sequence;
};

AudioProcessor::AudioProcessor(QObject *parent)
    : QObject{parent},m_impl(new Impl)
{

}

AudioProcessor::~AudioProcessor()
{
    delete m_impl;
}

void AudioProcessor::init(Sequence *t_sequence)
{
    m_impl->sequence = t_sequence;
}

void AudioProcessor::startProcessing()
{

}

void AudioProcessor::processingComplete()
{

}

void AudioProcessor::complete()
{
    processingComplete();
    emit completed();
}

Sequence *AudioProcessor::sequence() const
{
    return m_impl->sequence;
}

QUrl AudioProcessor::audioFilePath() const
{
    return QUrl::fromLocalFile( m_impl->sequence->filePath());
}


} // namespace photon
