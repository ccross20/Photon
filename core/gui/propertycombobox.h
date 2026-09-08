#ifndef PHOTON_PROPERTYCOMBOBOX_H
#define PHOTON_PROPERTYCOMBOBOX_H

#include <QComboBox>
#include "photon-global.h"

namespace photon {

// A QComboBox forced to grow to fill its form column the same way the
// surrounding line edits and spin boxes do (see the constructor) - some
// styles, macOS's included, give QComboBox a Fixed horizontal size policy
// that QFormLayout::AllNonFixedFieldsGrow skips over, leaving it at its own
// (content-driven) sizeHint width instead.
//
// Deliberately does *not* touch popup sizing/geometry - several attempts at
// that (forcing the classic popup mode, custom item padding, resizing the
// popup frame after showPopup(), pre-sizing the view before it) each just
// traded one popup-height bug for another. The popup is left to Qt's own
// default sizing.
class PHOTONCORE_EXPORT PropertyComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit PropertyComboBox(QWidget *parent = nullptr);
};

} // namespace photon

#endif // PHOTON_PROPERTYCOMBOBOX_H
