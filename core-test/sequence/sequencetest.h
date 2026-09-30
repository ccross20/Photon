#ifndef PHOTON_SEQUENCETEST_H
#define PHOTON_SEQUENCETEST_H

#include <QObject>

namespace photon {

class SequenceTest : public QObject
{
    Q_OBJECT
public:
    explicit SequenceTest(QObject *parent = nullptr);

private slots:
    void simpleProcess();
    void beatPulseTimes();
    void clipModelStaysConsistent();

signals:

};

} // namespace photon

#endif // PHOTON_SEQUENCETEST_H
