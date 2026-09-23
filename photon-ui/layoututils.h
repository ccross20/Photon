#ifndef PHOTONUI_LAYOUTUTILS_H
#define PHOTONUI_LAYOUTUTILS_H

#include <QLayout>
#include <QFormLayout>
#include <QWidget>

// Generic layout teardown helpers. These live here rather than in core's
// util/utils.h so photon-ui widgets can use them without depending on core;
// core/util/utils.h includes this header, so existing callers are unaffected.
//
// Header-only statics, matching how util/utils.h has always exposed them.

static void clearLayout(QLayout *layout)
{
    while (layout->count()>0) {
        auto item = layout->takeAt(0);
        delete item->widget();
        if(item->layout())
            clearLayout(item->layout());
    }
}

static void clearFormLayout(QFormLayout *layout)
{
    while (layout->rowCount()>0) {
        layout->removeRow(0);
    }
}

static void removeAllFromLayout(QLayout *layout)
{
    while (layout->count()>0) {
        auto item = layout->takeAt(0);
        if(item->layout())
            removeAllFromLayout(item->layout());
    }
}

static void clearWidget(QWidget *widget)
{
    for(QObject *obj : widget->children())
        delete obj;
}

#endif // PHOTONUI_LAYOUTUTILS_H
