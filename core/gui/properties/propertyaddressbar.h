#ifndef PHOTON_PROPERTYADDRESSBAR_H
#define PHOTON_PROPERTYADDRESSBAR_H

#include <QWidget>
#include "photon-global.h"
#include "propertyaddress.h"

class QHBoxLayout;

namespace photon {

// Clickable breadcrumbs for the address of whatever is being edited -
// "Routine > Canvas Subgraph > Noise", "Main Surface > Fader 1",
// "Rig > Truss > Mover 3".
//
// Same visual language as GraphWidget's graph breadcrumbs, which this
// generalises: every segment but the last navigates to that ancestor, the last
// is shown plain because it is already what's on screen.
class PHOTONCORE_EXPORT PropertyAddressBar : public QWidget
{
    Q_OBJECT
public:
    explicit PropertyAddressBar(QWidget *parent = nullptr);

    void setAddress(const PropertyAddress &address);
    const PropertyAddress &address() const { return m_address; }

signals:
    // A breadcrumb was clicked. The panel decides what selecting that ancestor
    // means - opening a graph, selecting a scene object - since navigation is
    // domain behaviour, not the bar's.
    void segmentActivated(const photon::PropertyAddress::Segment &segment, int index);

private:
    void rebuild();

    QHBoxLayout     *m_layout = nullptr;
    PropertyAddress  m_address;
};

} // namespace photon

#endif // PHOTON_PROPERTYADDRESSBAR_H
