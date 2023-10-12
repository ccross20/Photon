#ifndef PHOTON_CANVASCLIP_H
#define PHOTON_CANVASCLIP_H
#include "clip.h"


namespace photon {


class PHOTONCORE_EXPORT CanvasClip : public Clip
{
    Q_OBJECT
public:
    CanvasClip();

    CanvasClip(double start, double duration, QObject *parent = nullptr);
    ~CanvasClip();

    void processChannels(ProcessContext &) override;

    void addPixelLayout(PixelLayout *);
    void removePixelLayout(PixelLayout *);
    PixelLayout *pixelLayoutAtIndex(int index) const;
    int pixelLayoutCount() const;


    QVector<PixelSource*> sources() const;

    void restore(Project &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

    static ClipInformation info();

signals:

    void pixelLayoutAdded(photon::PixelLayout *);
    void pixelLayoutRemoved(photon::PixelLayout *);

private:
    class Impl;
    Impl *m_impl;
};


} // namespace photon

#endif // PHOTON_CANVASCLIP_H
