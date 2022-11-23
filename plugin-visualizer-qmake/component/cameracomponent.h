#ifndef PHOTON_CAMERACOMPONENT_H
#define PHOTON_CAMERACOMPONENT_H

#include "component/abstractcomponent.h"

namespace photon {

class CameraComponent : public AbstractComponent
{
public:
    CameraComponent();

    void setSize(const QSizeF &size);
    QSizeF size() const;

    void setZoom(double zoom);
    double zoom() const;

private:
    QSizeF m_size;
    double m_zoom;
};

} // namespace photon

#endif // PHOTON_CAMERACOMPONENT_H
