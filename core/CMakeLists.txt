cmake_minimum_required(VERSION 3.5)

project(photon-core VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS
    3DCore
    3DRender
    3DInput
    3DLogic
    3DExtras
    3DAnimation
)

set(PROJECT_SOURCES
        photon-global.h
        parametereditor.h
        parametereditor.cpp
        data/dmxmatrix.h
        data/dmxmatrix.cpp
        falloff/constantfalloffeffect.h
        falloff/constantfalloffeffect.cpp
        falloff/falloffeffect.h
        falloff/falloffeffect_p.h
        falloff/falloffeffect.cpp
        falloff/falloffmap2d.h
        falloff/falloffmap2d.cpp
        falloff/fixturefalloffmap.h
        falloff/fixturefalloffmap.cpp
        fixture/fixture.h
        fixture/fixture.cpp
        fixture/fixturechannel.h
        fixture/fixturechannel.cpp
        fixture/fixturecollection.h
        fixture/fixturecollection.cpp
        fixture/fixtureeditorwidget.h
        fixture/fixtureeditorwidget.cpp
        fixture/fixturemask.h
        fixture/fixturemask.cpp
        fixture/capability/fixturecapability.h
        fixture/capability/fixturecapability.cpp
        fixture/capability/nofunctioncapability.h
        fixture/capability/nofunctioncapability.cpp
        fixture/capability/pancapability.h
        fixture/capability/pancapability.cpp
        fixture/capability/shutterstrobecapability.h
        fixture/capability/shutterstrobecapability.cpp
        fixture/capability/tiltcapability.h
        fixture/capability/tiltcapability.cpp
        graph/bus/busevaluator.h
        graph/bus/busevaluator.cpp
        graph/bus/busgraph.h
        graph/bus/busgraph.cpp
        graph/bus/dmxgeneratematrixnode.h
        graph/bus/dmxgeneratematrixnode.cpp
        graph/bus/dmxwriternode.h
        graph/bus/dmxwriternode.cpp
        graph/bus/sequencenode.h
        graph/bus/sequencenode.cpp
        graph/node/math/arithmeticnode.h
        graph/node/math/arithmeticnode.cpp
        graph/node/math/trigonometrynode.h
        graph/node/math/trigonometrynode.cpp
        graph/parameter/colorparameter.h
        graph/parameter/colorparameter.cpp
        graph/parameter/dmxmatrixparameter.h
        graph/parameter/dmxmatrixparameter.cpp
        gui/color/colorslider.h
        gui/color/colorslider.cpp
        gui/color/colorwheelswatch.h
        gui/color/colorwheelswatch.cpp
        gui/color/colorselectorwidget.h
        gui/color/colorselectorwidget.cpp
        gui/color/colorselectordialog.h
        gui/color/colorselectordialog.cpp
        gui/color/huebox.h
        gui/color/huebox.cpp
        gui/color/simpleswatch.h
        gui/color/simpleswatch.cpp
        gui/dmxviewer/dmxviewer.h
        gui/dmxviewer/dmxviewer.cpp
        gui/dmxviewer/dmxviewercells.h
        gui/dmxviewer/dmxviewercells.cpp
        gui/gizmo/rectanglegizmo.h
        gui/gizmo/rectanglegizmo.cpp
        gui/fixturefalloff2deditor.h
        gui/fixturefalloff2deditor_p.h
        gui/fixturefalloff2deditor.cpp
        gui/guimanager.h
        gui/guimanager_p.h
        gui/guimanager.cpp
        gui/menufactory.h
        gui/menufactory.cpp
        gui/panel.h
        gui/panel_p.h
        gui/panel.cpp
        gui/panel/buspanel.h
        gui/panel/buspanel.cpp
        gui/panel/canvascollectionpanel.h
        gui/panel/canvascollectionpanel_p.h
        gui/panel/canvascollectionpanel.cpp
        gui/panel/dmxviewerpanel.h
        gui/panel/dmxviewerpanel.cpp
        gui/panel/fixturecollectionpanel.h
        gui/panel/fixturecollectionpanel_p.h
        gui/panel/fixturecollectionpanel.cpp
        gui/panel/routinecollectionpanel.h
        gui/panel/routinecollectionpanel_p.h
        gui/panel/routinecollectionpanel.cpp
        gui/panel/routineeditpanel.h
        gui/panel/routineeditpanel.cpp
        gui/panel/sequencecollectionpanel.h
        gui/panel/sequencecollectionpanel_p.h
        gui/panel/sequencecollectionpanel.cpp
        gui/panel/sequencepanel.h
        gui/panel/sequencepanel.cpp
        gui/vector3edit.h
        gui/vector3edit.cpp
        gui/window.h
        gui/window.cpp
        photoncore.h
        photoncore.cpp
        project/project.h
        project/project.cpp
        routine/node/fixturewriternode.h
        routine/node/fixturewriternode.cpp
        routine/node/globalsnode.h
        routine/node/globalsnode.cpp
        routine/node/numberinputnode.h
        routine/node/numberinputnode.cpp
        routine/node/colorinputnode.h
        routine/node/colorinputnode.cpp
        routine/routineevaluationcontext.h
        routine/routine.h
        routine/routine.cpp
        routine/routinecollection.h
        routine/routinecollection.cpp
        plugin/iplugin.h
        pixel/canvas.h
        pixel/canvas.cpp
        pixel/canvascollection.h
        pixel/canvascollection.cpp
        plugin/pluginfactory.h
        plugin/pluginfactory.cpp
        sequence/canvaslayergroup.h
        sequence/canvaslayergroup.cpp
        sequence/channel.h
        sequence/channel_p.h
        sequence/channel.cpp
        sequence/channeleffect.h
        sequence/channeleffect_p.h
        sequence/channeleffect.cpp
        sequence/clip.h
        sequence/clip_p.h
        sequence/clip.cpp
        sequence/cliplayer.h
        sequence/cliplayer_p.h
        sequence/cliplayer.cpp
        sequence/constantchanneleffect.h
        sequence/constantchanneleffect.cpp
        sequence/layer.h
        sequence/layer_p.h
        sequence/layer.cpp
        sequence/layergroup.h
        sequence/layergroup_p.h
        sequence/layergroup.cpp
        sequence/masterlayerchanneleffect.h
        sequence/masterlayerchanneleffect.cpp
        sequence/masterlayer.h
        sequence/masterlayer.cpp
        sequence/sequence.h
        sequence/sequence_p.h
        sequence/sequence.cpp
        sequence/sequencecollection.h
        sequence/sequencecollection.cpp
        sequence/spline.h
        sequence/spline.cpp
        sequence/track.h
        sequence/track.cpp
        sequence/viewer/clipmodel.h
        sequence/viewer/clipmodel.cpp
        sequence/viewer/sequenceclip.h
        sequence/viewer/sequenceclip.cpp
        sequence/viewer/channeleffecteditor.h
        sequence/viewer/channeleffecteditor_p.h
        sequence/viewer/channeleffecteditor.cpp
        sequence/viewer/clipstructureviewer.h
        sequence/viewer/clipstructureviewer.cpp
        sequence/viewer/clipeffecteditor.h
        sequence/viewer/clipeffecteditor.cpp
        sequence/viewer/layeritem.h
        sequence/viewer/layeritem.cpp
        sequence/viewer/stackedparameterwidget.h
        sequence/viewer/stackedparameterwidget.cpp
        sequence/viewer/timebar.h
        sequence/viewer/timebar.cpp
        sequence/viewer/timelineheader.h
        sequence/viewer/timelineheader.cpp
        sequence/viewer/timelinecliplayer.h
        sequence/viewer/timelinecliplayer.cpp
        sequence/viewer/timelinelayergroup.h
        sequence/viewer/timelinelayergroup.cpp
        sequence/viewer/timelinemasterlayer.h
        sequence/viewer/timelinemasterlayer.cpp
        sequence/viewer/timelinescene.h
        sequence/viewer/timelinescene.cpp
        sequence/viewer/timelineviewer.h
        sequence/viewer/timelineviewer.cpp
        sequence/viewer/sequencewidget.h
        sequence/viewer/sequencewidget.cpp
        settings/resourcemanager.h
        settings/resourcemanager.cpp
        settings/settings.h
        settings/settings.cpp
        settings/settingspage.h
        settings/settingspage.cpp
        third-party/advanced-docking/ads_globals.cpp
        third-party/advanced-docking/DockAreaTabBar.cpp
        third-party/advanced-docking/DockAreaTitleBar.cpp
        third-party/advanced-docking/DockAreaWidget.cpp
        third-party/advanced-docking/DockComponentsFactory.cpp
        third-party/advanced-docking/DockContainerWidget.cpp
        third-party/advanced-docking/DockFocusController.cpp
        third-party/advanced-docking/DockingStateReader.cpp
        third-party/advanced-docking/DockManager.cpp
        third-party/advanced-docking/DockOverlay.cpp
        third-party/advanced-docking/DockSplitter.cpp
        third-party/advanced-docking/DockWidget.cpp
        third-party/advanced-docking/DockWidgetTab.cpp
        third-party/advanced-docking/ElidingLabel.cpp
        third-party/advanced-docking/FloatingDockContainer.cpp
        third-party/advanced-docking/FloatingDragPreview.cpp
        third-party/advanced-docking/IconProvider.cpp
        third-party/advanced-docking/linux/FloatingWidgetTitleBar.cpp
        timekeeper.h
        timekeeper.cpp
        viewport/geometry/Line.h
        viewport/geometry/Line.cpp
        viewport/geometry/Matrices.h
        viewport/geometry/Matrices.cpp
        viewport/geometry/Pipe.h
        viewport/geometry/Pipe.cpp
        viewport/geometry/Plane.h
        viewport/geometry/Plane.cpp
        viewport/geometry/Vectors.h
        viewport/geometry/trussgeometry.h
        viewport/geometry/trussgeometry.cpp
        viewport/gridplane.h
        viewport/gridplane.cpp
        viewport/scenemodifier.h
        viewport/scenemodifier.cpp
        viewport/visualizerscene.h
        viewport/visualizerscene.cpp
        resources.qrc
)

qt_add_resources(PROJECT_SOURCES third-party/advanced-docking/ads.qrc)

add_library(photon-core SHARED ${PROJECT_SOURCES})

target_include_directories(photon-core PUBLIC ../core)
target_include_directories(photon-core PUBLIC ../keira)

target_link_libraries(photon-core PRIVATE keira)

target_link_libraries(photon-core PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(photon-core PRIVATE
    Qt6::3DCore
    Qt6::3DRender
    Qt6::3DLogic
    Qt6::3DExtras
    Qt6::3DAnimation
)

target_compile_definitions(photon-core PRIVATE PHOTONCORE_LIBRARY)


