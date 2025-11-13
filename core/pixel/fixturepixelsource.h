#ifndef FIXTUREPIXELSOURCE_H
#define FIXTUREPIXELSOURCE_H
#include "pixelsource.h"


namespace photon {

class PHOTONCORE_EXPORT FixturePixelSource : public PixelSource
{
public:
    FixturePixelSource(const QVector<FixtureCapability *> &);
    virtual ~FixturePixelSource();

    const QVector<FixtureCapability *> &capabilities() const;
    Fixture *fixture() const;
    virtual int dmxOffset() const override;
    virtual int dmxSize() const override;
    virtual int universe() const override;

    const QVector<QPointF> &positions() const override;

    virtual void process(ProcessContext &, const QTransform &, double blend = 1.0) const override;

    virtual void readFromJson(const QJsonObject &, const LoadContext &) override;
    virtual void writeToJson(QJsonObject &) const override;

    QByteArray sourceUniqueId() const override;


private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // FIXTUREPIXELSOURCE_H
