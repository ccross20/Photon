#ifndef PHOTON_MAPPED2DFALLOFF_H
#define PHOTON_MAPPED2DFALLOFF_H
#include <QWidget>
#include "falloff/falloffeffect.h"
#include "falloff/falloffmap2d.h"
#include "gui/fixturefalloff2deditor.h"

namespace photon {

class Mapped2DFalloff;

class Mapped2DFalloffEditor : public QWidget
{
public:
    Mapped2DFalloffEditor(Mapped2DFalloff *);


private slots:
    void mapUpdated();

private:
    Mapped2DFalloff *m_effect;
    FixtureFalloff2DEditor *m_editor;
};


class Mapped2DFalloff : public FalloffEffect
{
public:
    Mapped2DFalloff();

    double falloff(Fixture *) const override;
    QWidget *createEditor() override;

    void setFalloff(const FalloffMap2D &map);
    FalloffMap2D falloff() const;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static FalloffEffectInformation info();

private:
    FalloffMap2D m_map;
};

} // namespace photon

#endif // PHOTON_MAPPED2DFALLOFF_H
