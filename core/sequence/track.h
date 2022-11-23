#ifndef PHOTON_TRACK_H
#define PHOTON_TRACK_H

#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT Track : public QObject
{
    Q_OBJECT
public:
    explicit Track(QObject *parent = nullptr);

signals:


private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TRACK_H
