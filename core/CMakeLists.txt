cmake_minimum_required(VERSION 3.5)

project(photon-core VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)
find_package(Qt6 REQUIRED COMPONENTS
    3DCore
    3DRender
    3DInput
    3DLogic
    3DExtras
    3DAnimation
    MultimediaWidgets
)

set(PROJECT_SOURCES
        photon-global.h
        audio/audioprocessor.h
        audio/audioprocessor.cpp
        parametereditor.h
        parametereditor.cpp
        canvas/canvaseffect.h
        canvas/canvaseffect.cpp
        channel/parameter/boolchannelparameter.h
        channel/parameter/boolchannelparameter.cpp
        channel/parameter/channelparameter.h
        channel/parameter/channelparameter.cpp
        channel/parameter/colorchannelparameter.h
        channel/parameter/colorchannelparameter.cpp
        channel/parameter/numberchannelparameter.h
        channel/parameter/numberchannelparameter.cpp
        channel/parameter/optionchannelparameter.h
        channel/parameter/optionchannelparameter.cpp
        channel/parameter/point2channelparameter.h
        channel/parameter/point2channelparameter.cpp
        channel/parameter/view/boolchannelparameterview.h
        channel/parameter/view/boolchannelparameterview.cpp
        channel/parameter/view/channelparameterview.h
        channel/parameter/view/channelparameterview.cpp
        channel/parameter/view/colorchannelparameterview.h
        channel/parameter/view/colorchannelparameterview.cpp
        channel/parameter/view/numberchannelparameterview.h
        channel/parameter/view/numberchannelparameterview.cpp
        channel/parameter/view/optionchannelparameterview.h
        channel/parameter/view/optionchannelparameterview.cpp
        channel/parameter/view/point2channelparameterview.h
        channel/parameter/view/point2channelparameterview.cpp
        channel/parameter/view/channelparameterwidget.h
        channel/parameter/view/channelparameterwidget.cpp
        data/dmxmatrix.h
        data/dmxmatrix.cpp
        data/types.h
        falloff/constantfalloffeffect.h
        falloff/constantfalloffeffect.cpp
        falloff/falloffeffect.h
        falloff/falloffeffect_p.h
        falloff/falloffeffect.cpp
        falloff/falloffmap2d.h
        falloff/falloffmap2d.cpp
        falloff/fixturefalloffmap.h
        falloff/fixturefalloffmap.cpp
        fixture/dmxreceiver.h
        fixture/fixture.h
        fixture/fixture.cpp
        fixture/fixturechannel.h
        fixture/fixturechannel_p.h
        fixture/fixturechannel.cpp
        fixture/fixturecollection.h
        fixture/fixturecollection.cpp
        fixture/fixtureeditorwidget.h
        fixture/fixtureeditorwidget.cpp
        fixture/fixturemask.h
        fixture/fixturemask.cpp
        fixture/fixturevirtualchannel.h
        fixture/fixturevirtualchannel.cpp
        fixture/fixturewheel.h
        fixture/fixturewheel.cpp
        fixture/maskeffect.h
        fixture/maskeffect_p.h
        fixture/maskeffect.cpp
        fixture/capability/anglecapability.h
        fixture/capability/anglecapability.cpp
        fixture/capability/capabilityfactory.h
        fixture/capability/capabilityfactory.cpp
        fixture/capability/colorcapability.h
        fixture/capability/colorcapability.cpp
        fixture/capability/colorintensitycapability.h
        fixture/capability/colorintensitycapability.cpp
        fixture/capability/dimmercapability.h
        fixture/capability/dimmercapability.cpp
        fixture/capability/fixturecapability.h
        fixture/capability/fixturecapability.cpp
        fixture/capability/focuscapability.h
        fixture/capability/focuscapability.cpp
        fixture/capability/nofunctioncapability.h
        fixture/capability/nofunctioncapability.cpp
        fixture/capability/pancapability.h
        fixture/capability/pancapability.cpp
        fixture/capability/shutterstrobecapability.h
        fixture/capability/shutterstrobecapability.cpp
        fixture/capability/tiltcapability.h
        fixture/capability/tiltcapability.cpp
        fixture/capability/wheelslotcapability.h
        fixture/capability/wheelslotcapability.cpp
        fixture/capability/zoomcapability.h
        fixture/capability/zoomcapability.cpp
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
        graph/parameter/canvasparameter.h
        graph/parameter/canvasparameter.cpp
        graph/parameter/colorparameter.h
        graph/parameter/colorparameter.cpp
        graph/parameter/dmxmatrixparameter.h
        graph/parameter/dmxmatrixparameter.cpp
        graph/parameter/fixtureparameter.h
        graph/parameter/fixtureparameter.cpp
        graph/parameter/pathparameter.h
        graph/parameter/pathparameter.cpp
        graph/parameter/vector3dparameter.h
        graph/parameter/vector3dparameter.cpp
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
        gui/dialog/canvasdialog.h
        gui/dialog/canvasdialog.cpp
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
        gui/panel/canvasviewerpanel.h
        gui/panel/canvasviewerpanel.cpp
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
        gui/panel/pixellayoutcollectionpanel.h
        gui/panel/pixellayoutcollectionpanel_p.h
        gui/panel/pixellayoutcollectionpanel.cpp
        gui/panel/sequencecollectionpanel.h
        gui/panel/sequencecollectionpanel_p.h
        gui/panel/sequencecollectionpanel.cpp
        gui/panel/sequencepanel.h
        gui/panel/sequencepanel.cpp
        gui/pixellayouteditor.h
        gui/pixellayouteditor_p.h
        gui/pixellayouteditor.cpp
        gui/pointedit.h
        gui/pointedit.cpp
        gui/sceneobjectselector.h
        gui/sceneobjectselector_p.h
        gui/sceneobjectselector.cpp
        gui/vector3edit.h
        gui/vector3edit.cpp
        gui/waveformwidget.h
        gui/waveformwidget.cpp
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
        routine/node/fixtureinfonode.h
        routine/node/fixtureinfonode.cpp
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
        pixel/pixellayout.h
        pixel/pixellayout.cpp
        pixel/pixellayoutcollection.h
        pixel/pixellayoutcollection.cpp
        pixel/pixelsource.h
        pixel/pixelsource.cpp
        pixel/pixelsourcelayout.h
        pixel/pixelsourcelayout.cpp
        pixel/pixelstrip.h
        pixel/pixelstrip.cpp
        plugin/pluginfactory.h
        plugin/pluginfactory.cpp
        scene/sceneiterator.h
        scene/scenemanager.h
        scene/scenemanager.cpp
        scene/scenemodel.h
        scene/scenemodel.cpp
        scene/scenegroup.h
        scene/scenegroup.cpp
        scene/scenefactory.h
        scene/scenefactory.cpp
        scene/sceneobject.h
        scene/sceneobject_p.h
        scene/sceneobject.cpp
        scene/truss.h
        scene/truss.cpp
        sequence/beatlayer.h
        sequence/beatlayer.cpp
        sequence/canvasclip.h
        sequence/canvasclip.cpp
        sequence/canvasclipeffect.h
        sequence/canvasclipeffect.cpp
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
        sequence/clipeffect.h
        sequence/clipeffect_p.h
        sequence/clipeffect.cpp
        sequence/cliplayer.h
        sequence/cliplayer_p.h
        sequence/cliplayer.cpp
        sequence/constantchanneleffect.h
        sequence/constantchanneleffect.cpp
        sequence/fixtureclip.h
        sequence/fixtureclip.cpp
        sequence/fixtureclipeffect.h
        sequence/fixtureclipeffect.cpp
        sequence/gradientchanneleffect.h
        sequence/gradientchanneleffect.cpp
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
        sequence/routineclip.h
        sequence/routineclip.cpp
        sequence/sequence.h
        sequence/sequence_p.h
        sequence/sequence.cpp
        sequence/sequencecollection.h
        sequence/sequencecollection.cpp
        sequence/stateclip.h
        sequence/stateclip.cpp
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
        sequence/viewer/sequencewaveformeditor.h
        sequence/viewer/sequencewaveformeditor.cpp
        sequence/viewer/sequencewidget.h
        sequence/viewer/sequencewidget.cpp
        sequence/viewer/stateeditor.h
        sequence/viewer/stateeditor.cpp
        sequence/viewer/waveformheader.h
        sequence/viewer/waveformheader_p.h
        sequence/viewer/waveformheader.cpp
        settings/resourcemanager.h
        settings/resourcemanager.cpp
        settings/settings.h
        settings/settings.cpp
        settings/settingspage.h
        settings/settingspage.cpp
        state/colorstate.h
        state/colorstate.cpp
        state/colorwheelstate.h
        state/colorwheelstate.cpp
        state/dimmerstate.h
        state/dimmerstate.cpp
        state/focusstate.h
        state/focusstate.cpp
        state/panstate.h
        state/panstate.cpp
        state/stateevaluationcontext.h
        state/shutterstate.h
        state/shutterstate.cpp
        state/state.h
        state/state.cpp
        state/statecapability.h
        state/statecapability.cpp
        state/tiltanglestate.h
        state/tiltanglestate.cpp
        state/tiltanglecenteredstate.h
        state/tiltanglecenteredstate.cpp
        state/tiltstate.h
        state/tiltstate.cpp
        state/zoomstate.h
        state/zoomstate.cpp
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
        third-party/advanced-docking/AutoHideSideBar.cpp
        third-party/advanced-docking/AutoHideTab.cpp
        third-party/advanced-docking/AutoHideDockContainer.cpp
        third-party/advanced-docking/PushButton.cpp
        third-party/advanced-docking/ResizeHandle.cpp
        third-party/advanced-docking/linux/FloatingWidgetTitleBar.cpp
        third-party/fastnoise.h
        third-party/fastnoise.cpp
        timekeeper.h
        timekeeper.cpp
        util/noisegenerator.h
        util/noisegenerator.cpp
        util/utils.h
        util/utils.cpp
        resources.qrc
)

qt_add_resources(PROJECT_SOURCES third-party/advanced-docking/ads.qrc)

add_library(photon-core SHARED ${PROJECT_SOURCES}
    state/statecollection.h state/statecollection.cpp
    tag/tagcollection.h tag/tagcollection.cpp
    gui/panel/tagcollectionpanel.h gui/panel/tagcollectionpanel.cpp
    gui/panel/tagcollectionpanel_p.h
    gui/tagpicker.h gui/tagpicker.cpp
    gui/tageditwidget.h gui/tageditwidget.cpp
    gui/tageditwidget_p.h
    scene/scenearrow.h scene/scenearrow.cpp
    state/gobostate.h state/gobostate.cpp
    fixture/capability/wheelshakecapability.h fixture/capability/wheelshakecapability.cpp
    fixture/capability/wheelrotationcapability.h fixture/capability/wheelrotationcapability.cpp
    fixture/capability/wheelslotrotationcapability.h fixture/capability/wheelslotrotationcapability.cpp
    state/wheelrotationstate.h state/wheelrotationstate.cpp
    state/wheelslotrotationstate.h state/wheelslotrotationstate.cpp
    state/slotstate.h state/slotstate.cpp
    data/spline.cpp data/spline.h
    util/BezierUtils.h
    util/mathutils.h
    channel/splinechanneleffect.h channel/splinechanneleffect.cpp)

target_include_directories(photon-core PUBLIC ../core)
target_include_directories(photon-core PUBLIC ../keira)
target_include_directories(photon-core PUBLIC third-party/advanced-docking)

target_link_libraries(photon-core PRIVATE keira)

target_link_libraries(photon-core PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(photon-core PRIVATE
    Qt6::3DCore
    Qt6::3DRender
    Qt6::3DLogic
    Qt6::3DExtras
    Qt6::3DAnimation
    Qt6::MultimediaWidgets
    Qt6::Network
)

target_compile_definitions(photon-core PRIVATE PHOTONCORE_LIBRARY)


