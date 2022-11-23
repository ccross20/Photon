#ifndef PHOTON_FIXTUREFALLOFF2DEDITOR_H
#define PHOTON_FIXTUREFALLOFF2DEDITOR_H
#include <QWidget>
#include "photon-global.h"
#include "falloff/falloffmap2d.h"

namespace photon {

class PHOTONCORE_EXPORT FixtureFalloff2DEditor : public QWidget
{
    Q_OBJECT
public:
    FixtureFalloff2DEditor();
    ~FixtureFalloff2DEditor();

    void setMap(const FalloffMap2D &);
    FalloffMap2D map() const;

protected:
    void resizeEvent(QResizeEvent *) override;

private slots:
    void sceneChanged(const QList<QRectF> &regions);

signals:
    void mapUpdated();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTUREFALLOFF2DEDITOR_H
