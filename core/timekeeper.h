#ifndef PHOTON_TIMEKEEPER_H
#define PHOTON_TIMEKEEPER_H

#include <QObject>

namespace photon {

class Timekeeper : public QObject
{
    Q_OBJECT
public:
    explicit Timekeeper(QObject *parent = nullptr);
    ~Timekeeper();

    void setTickRate(double);

private slots:
    void timeout();

signals:
    void tick();
    void postTick();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TIMEKEEPER_H
