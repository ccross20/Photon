#include <QAbstractItemView>
#include <QScreen>
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
    setMinimumWidth(150);

    // The app stylesheet gives these combos a non-native border, and Qt's
    // stylesheet style reads that as "use the menu-style popup": it floats
    // over the combo centred on the current item, at its own width rather
    // than the combo's, and grows scroller arrows instead of showing every
    // row. That's what makes the popup ignore any geometry set on it - the
    // scrollers re-lay it out. combobox-popup: 0 puts it back on the plain
    // list popup, which anchors under the combo and has no scrollers to
    // fight; showPopup() below then fixes its height.
    setStyleSheet(QStringLiteral("QComboBox { combobox-popup: 0; }"));
}

void PropertyComboBox::showPopup()
{
    QComboBox::showPopup();

    QAbstractItemView *list = view();
    QWidget *container = list ? list->parentWidget() : nullptr;
    if(!list || !container || count() < 1)
        return;

    // Qt sizes the popup from maxVisibleItems and its own row-height guess,
    // which under a stylesheet comes up a row short - measure the rows
    // instead, so the popup is exactly as tall as its contents whenever
    // there's screen room for it.
    int contentHeight = 0;
    for(int row = 0; row < count(); ++row)
    {
        const int rowHeight = list->sizeHintForRow(row);
        contentHeight += rowHeight > 0 ? rowHeight : list->fontMetrics().height();
    }

    // Whatever the container and the view draw around those rows - measured
    // rather than assumed, since it's style-dependent.
    const int chrome = qMax(0, container->height() - list->height()) + (2 * list->frameWidth());
    const int desiredHeight = contentHeight + chrome;

    const QPoint comboTopLeft = mapToGlobal(QPoint(0, 0));
    const QRect available = screen()->availableGeometry();
    const int spaceBelow = available.bottom() - (comboTopLeft.y() + height());
    const int spaceAbove = comboTopLeft.y() - available.top();

    // A property form's field column can be wider than the panel scrolling
    // it, which leaves the combo itself clipped at the panel edge. Matching
    // the combo's full width would then spill the popup out over whatever is
    // beside the panel, so track the part of the combo actually on screen.
    const QRect visibleRect = visibleRegion().boundingRect();
    const int popupX = comboTopLeft.x() + (visibleRect.isEmpty() ? 0 : visibleRect.left());
    int popupWidth = visibleRect.isEmpty() ? width() : visibleRect.width();
    popupWidth = qMin(popupWidth, available.right() - popupX + 1);

    int popupHeight = desiredHeight;
    int popupY = comboTopLeft.y() + height();

    if(desiredHeight > spaceBelow)
    {
        if(desiredHeight <= spaceAbove)
        {
            popupY = comboTopLeft.y() - desiredHeight;   // flip above the combo
        }
        else if(spaceAbove > spaceBelow)
        {
            popupHeight = spaceAbove;                    // taller side wins, and scrolls
            popupY = available.top();
        }
        else
        {
            popupHeight = spaceBelow;
        }
    }

    const bool everythingFits = popupHeight >= desiredHeight;
    list->setVerticalScrollBarPolicy(everythingFits ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);

    // Left edge and width tracking the combo itself, so the popup reads as
    // belonging to it rather than as a free-floating menu.
    container->setGeometry(popupX, popupY, popupWidth, popupHeight);

    // QComboBox scrolls the current item into view before this ran, against
    // the old (short) height - with everything visible that just leaves the
    // list scrolled past its first rows.
    if(everythingFits)
        list->scrollToTop();
}

} // namespace photon
