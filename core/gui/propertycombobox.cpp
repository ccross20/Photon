#include "propertycombobox.h"

namespace photon {

PropertyComboBox::PropertyComboBox(QWidget *parent) : QComboBox(parent)
{
    // Some styles (macOS's included) give QComboBox a Fixed horizontal size
    // policy, which QFormLayout::AllNonFixedFieldsGrow explicitly excludes
    // from growing to fill the shared field column - so it's left at its own
    // sizeHint, which QComboBox derives from its widest item text and can
    // come out a few pixels wider *or* narrower than the neighboring fields
    // that did grow to fill the column. Forcing Expanding here makes it
    // follow the same rule as every other field.
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

} // namespace photon
