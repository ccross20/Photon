#ifndef PHOTON_TRACEPATHCLIP_H
#define PHOTON_TRACEPATHCLIP_H

#include <QWidget>
#include "sequence/clipeffect.h"

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

class TracePathClip : public ClipEffect
{
public:
    TracePathClip();

    void init() override;
    void evaluateFixture(ClipEffectEvaluationContext &) const override;
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
