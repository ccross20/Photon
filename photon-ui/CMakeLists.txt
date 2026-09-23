cmake_minimum_required(VERSION 3.5)

project(photon-ui VERSION 0.1 LANGUAGES CXX)

# The app's widget toolkit: generic Qt Widgets controls with no domain
# knowledge. Sits below both keira and photon-core so the two can share
# controls - see photon-ui-global.h for the rule this target must keep.

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    photon-ui-global.h
    layoututils.h

    # Shared property editing: the controls and the page layout every
    # property editor in the app is built from.
    propertywidgets.h propertywidgets.cpp

    # Numeric / geometry fields
    numberscrubfield.h numberscrubfield.cpp
    pointedit.h pointedit.cpp
    xypad.h xypad.cpp
    vector3edit.h vector3edit.cpp

    # General controls
    propertycombobox.h propertycombobox.cpp
    flowlayout.h flowlayout.cpp

    # Colour picking
    color/colorslider.h color/colorslider.cpp
    color/huebox.h color/huebox.cpp
    color/simpleswatch.h color/simpleswatch.cpp
    color/colorselectorwidget.h color/colorselectorwidget.cpp
    color/colorselectordialog.h color/colorselectordialog.cpp
    color/colorwheelswatch.h color/colorwheelswatch.cpp

    # Tag chips
    tag/tagcolor.h tag/tagcolor.cpp
    tag/tagmime.h tag/tagmime.cpp
    tag/tagchip.h tag/tagchip.cpp
    tag/tageditorwidget.h tag/tageditorwidget.cpp

    # On-canvas manipulation handles
    gizmo/gizmogroup.h gizmo/gizmogroup.cpp
    gizmo/gizmohandle.h gizmo/gizmohandle.cpp
    gizmo/rectanglegizmo.h gizmo/rectanglegizmo.cpp
)

add_library(photon-ui SHARED ${PROJECT_SOURCES})

target_include_directories(photon-ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(photon-ui PUBLIC Qt${QT_VERSION_MAJOR}::Widgets)
target_compile_definitions(photon-ui PRIVATE PHOTONUI_LIBRARY)
