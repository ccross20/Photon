#ifndef PHOTON_CANVASCLIP_H
#define PHOTON_CANVASCLIP_H
#include "clip.h"


namespace photon {

/*
class CanvasClip : public Clip
{
public:
    CanvasClip();

    CanvasClip(double start, double duration, QObject *parent = nullptr);
    ~CanvasClip();

    void processChannels(ProcessContext &) override;



    QString name() const override;

    void restore(Project &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};
*/

} // namespace photon

#endif // PHOTON_CANVASCLIP_H
