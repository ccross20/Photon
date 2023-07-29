#ifndef PHOTON_TRACEPATHCLIP_H
#define PHOTON_TRACEPATHCLIP_H

#include <QWidget>
#include "sequence/fixtureclipeffect.h"

namespace photon {

class TracePathClip;

class TracePathWidget : public QWidget
{
    Q_OBJECT
public:
    TracePathWidget(TracePathClip *);

private slots:
    void updateRadius(double);
    void updateOffset(double);

private:
    TracePathClip *m_effect;
};

class TracePathClip : public FixtureClipEffect
{
public:
    TracePathClip();

    void init() override;
    void evaluate(FixtureClipEffectEvaluationContext &) const override;
    QWidget *createEditor() override;
    static ClipEffectInformation info();
    void autoAssignFixtures(double);
    QPainterPath generatePath(double) const;

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

    QVector<Fixture *> fixtures;
    double radius = 1.5;
    double offset = 0.0;
};

} // namespace photon

#endif // PHOTON_TRACEPATHCLIP_H
