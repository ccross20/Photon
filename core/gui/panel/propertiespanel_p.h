#ifndef PROPERTIESPANEL_P_H
#define PROPERTIESPANEL_P_H

#include <QVBoxLayout>
#include <QLabel>
#include "propertiespanel.h"

namespace photon {

class PropertiesPanel::Impl
{
public:
    QVBoxLayout *layout;
    QLabel *emptyLabel;
    QWidget *editorWidget = nullptr;
};

}

#endif // PROPERTIESPANEL_P_H
