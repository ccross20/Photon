#ifndef PHOTON_BUSEVALUATOR_H
#define PHOTON_BUSEVALUATOR_H

#include <QObject>
#include "busgraph.h"

namespace keira { class GraphEvaluator; }

namespace photon {

class PHOTONCORE_EXPORT BusEvaluator : public QObject
{
    Q_OBJECT
public:
    explicit BusEvaluator(QObject *parent = nullptr);
    ~BusEvaluator();

    void setBus(BusGraph *);
    BusGraph *bus();

    DMXMatrix dmxMatrix() const;

    keira::GraphEvaluator *graphEvaluator() const;

public slots:
    void setBpm(double bpm);
    void tap();

signals:
    void evaluationCompleted();

private slots:
    void onFrameComplete();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_BUSEVALUATOR_H
