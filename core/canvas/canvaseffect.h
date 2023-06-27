#ifndef PHOTON_CANVASEFFECT_H
#define PHOTON_CANVASEFFECT_H
#include "photon-global.h"


namespace photon {

class PHOTONCORE_EXPORT CanvasEffect : public QObject
{
    Q_OBJECT
public:
    CanvasEffect();
    /*
    ~CanvasEffect();

    QByteArray uniqueId() const;

    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

private:
    class Impl;
    Impl *m_impl;
    */
};

} // namespace photon

#endif // PHOTON_CANVASEFFECT_H
