#ifndef FIXTUREPIXELSOURCE_H
#define FIXTUREPIXELSOURCE_H
#include "pixelsource.h"


namespace photon {

class PHOTONCORE_EXPORT FixturePixelSource : public PixelSource
{
public:
    FixturePixelSource(FixtureCapability *);
    virtual ~FixturePixelSource();

    FixtureCapability *capability() const;
    Fixture *fixture() const;
    virtual int dmxOffset() const override;
    virtual int dmxSize() const override;
    virtual int universe() const override;

    const QVector<QPointF> &positions() const override;

    virtual void process(ProcessContext &, const QTransform &) const override;

    virtual void readFromJson(const QJsonObject &, const LoadContext &) override;
    virtual void writeToJson(QJsonObject &) const override;

    QByteArray sourceUniqueId() const override;


private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // FIXTUREPIXELSOURCE_H
