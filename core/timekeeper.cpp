#include <QTimer>
#include "timekeeper.h"

namespace photon {

class Timekeeper::Impl
{
public:
    Impl(Timekeeper*);
    QTimer timer;
    Timekeeper *facade;
};

Timekeeper::Impl::Impl(Timekeeper *t_facade):facade(t_facade)
{
    //timer.setTimerType(Qt::TimerType::PreciseTimer);
    timer.setInterval(1000/30.0);
    timer.start();
}

Timekeeper::Timekeeper(QObject *parent)
    : QObject{parent},m_impl(new Impl(this))
{
    connect(&m_impl->timer, &QTimer::timeout, this, &Timekeeper::timeout);
}

Timekeeper::~Timekeeper()
{
    delete m_impl;
}

void Timekeeper::timeout()
{
    emit tick();
    emit postTick();
}

void Timekeeper::setTickRate(double t_rate)
{
    m_impl->timer.setInterval(t_rate);
}

} // namespace photon
