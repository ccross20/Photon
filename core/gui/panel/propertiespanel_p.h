#ifndef PROPERTIESPANEL_P_H
#define PROPERTIESPANEL_P_H

#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QToolButton>
#include "propertiespanel.h"
#include "gui/properties/propertyaddressbar.h"
#include "gui/properties/propertytabbar.h"

namespace photon {

class PropertiesPanel::Impl
{
public:
    PropertyAddressBar *addressBar = nullptr;
    PropertyTabBar     *tabBar = nullptr;
    QToolButton        *pinButton = nullptr;
    QScrollArea        *scroll = nullptr;
    QLabel             *emptyLabel = nullptr;
    QWidget            *editorWidget = nullptr;   // owned by the scroll area

    // Key under which pinned tabs round-trip through the project file.
    static const QByteArray UiStateKey;
};

}

#endif // PROPERTIESPANEL_P_H
