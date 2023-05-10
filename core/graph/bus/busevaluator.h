#ifndef PHOTON_BUSEVALUATOR_H
#define PHOTON_BUSEVALUATOR_H

#include <QObject>
#include "busgraph.h"

namespace photon {

class PHOTONCORE_EXPORT BusEvaluator : public QObject
{
    Q_OBJECT
public:
    explicit BusEvaluator(QObject *parent = nullptr);
    ~BusEvaluator();

    void setBus(BusGraph *);
    BusGraph *bus();
    double elapsed() const;

    const DMXMatrix &dmxMatrix() const;

public slots:
    void evaluate();

signals:

    void evaluationCompleted();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_BUSEVALUATOR_H
