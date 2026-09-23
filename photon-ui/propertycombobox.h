#ifndef PHOTON_PROPERTYCOMBOBOX_H
#define PHOTON_PROPERTYCOMBOBOX_H

#include <QComboBox>
#include "photon-ui-global.h"

namespace photon {

// A QComboBox forced to grow to fill its form column the same way the
// surrounding line edits and spin boxes do (see the constructor) - some
// styles, macOS's included, give QComboBox a Fixed horizontal size policy
// that QFormLayout::AllNonFixedFieldsGrow skips over, leaving it at its own
// (content-driven) sizeHint width instead.
//
// It also takes over popup geometry, which needs both halves to behave:
// the constructor puts the popup back on Qt's plain list style (the app
// stylesheet otherwise forces the menu style, which floats over the combo at
// its own width), and showPopup() then sizes it from the measured row
// heights so every item is visible when there's screen room for it.
class PHOTONUI_EXPORT PropertyComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit PropertyComboBox(QWidget *parent = nullptr);

    void showPopup() override;
};

} // namespace photon

#endif // PHOTON_PROPERTYCOMBOBOX_H
