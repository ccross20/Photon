#include "cameracomponent.h"

namespace photon {

CameraComponent::CameraComponent()
{

}

void CameraComponent::setSize(const QSizeF &t_size)
{
    m_size = t_size;
}

QSizeF CameraComponent::size() const
{
    return m_size;
}


void CameraComponent::setZoom(double t_zoom)
{
    m_zoom = t_zoom;
}

double CameraComponent::zoom() const
{
    return m_zoom;
}

} // namespace photon
