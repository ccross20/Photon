#ifndef PHOTONUI_GLOBAL_H
#define PHOTONUI_GLOBAL_H

#include <QtCore/qglobal.h>

// The widgets moved here were written against photon-global.h, which acted as
// an umbrella for these common Qt value types. Mirrored so they keep compiling
// without every file re-listing them.
#include <QDebug>
#include <QVariant>
#include <QPointF>
#include <QVector3D>
#include <QEvent>

// photon-ui is the app's widget toolkit: generic Qt Widgets controls with no
// knowledge of fixtures, projects, graphs or any other domain type.
//
// It sits below both keira and photon-core so the two can share controls. That
// matters because properties are edited in three different places (node
// parameters, surface gizmos, project resources) and they must look and behave
// the same - before this target existed, NumberScrubField lived in keira while
// PointEdit and XYPad lived in core, so neither side could use the other's.
//
// Nothing here may include photon-global.h, keira-global.h, or any header from
// core/ or keira/. If a widget needs a domain type it belongs in core instead.
#if defined(PHOTONUI_LIBRARY)
#  define PHOTONUI_EXPORT Q_DECL_EXPORT
#else
#  define PHOTONUI_EXPORT Q_DECL_IMPORT
#endif

#endif // PHOTONUI_GLOBAL_H
