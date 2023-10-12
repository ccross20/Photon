#ifndef PHOTON_AUDIOPROCESSOR_H
#define PHOTON_AUDIOPROCESSOR_H

#include <QObject>
#include "photon-global.h"

namespace photon {

struct AudioProcessorInformation
{

    AudioProcessorInformation(){}
    AudioProcessorInformation(std::function<AudioProcessor*()> _callback):callback(_callback){}

    QByteArray id;
    QByteArray translateId;
    QByteArray iconId;
    QString name;
    std::function<AudioProcessor*()> callback;

    CategoryList categories;
};

class PHOTONCORE_EXPORT AudioProcessor : public QObject
{
    Q_OBJECT
public:
    explicit AudioProcessor(QObject *parent = nullptr);
    virtual ~AudioProcessor();

    virtual void init(Sequence *);
    virtual void startProcessing();
    virtual void processingComplete();

    Sequence *sequence() const;
    QUrl audioFilePath() const;

signals:
    void completed();
    void error();

public slots:
    void complete();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_AUDIOPROCESSOR_H
