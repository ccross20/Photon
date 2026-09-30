cmake_minimum_required(VERSION 3.14)

project(plugin-visualizer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Gui GuiPrivate Widgets ShaderTools Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui GuiPrivate Widgets ShaderTools Multimedia)

set(SOURCES
    rhi/rhicamera.h
    rhi/rhicamera.cpp
    rhi/rhimesh.h
    rhi/rhimesh.cpp
    rhi/rhimodel.h
    rhi/rhimodel.cpp
    rhi/rhirenderer.h
    rhi/rhirenderer.cpp
    rhi/rhigizmo.h
    rhi/rhigizmo.cpp
    rhi/rhiwindow.h
    rhi/rhiwindow.cpp
    rhi/rhiviewport.h
    rhi/rhiviewport.cpp
    pluginvisualizer.cpp
    pluginvisualizer.h
    visualizerpanel.h
    visualizerpanel.cpp
    laserpreview.h
    laserpreview.cpp
)

qt_add_resources(SOURCES resources.qrc)
add_library(plugin-visualizer SHARED ${SOURCES})

qt6_add_shaders(plugin-visualizer "visualizer_shaders"
    PREFIX "/visualizer/shaders"
    BASE "rhi/shaders"
    FILES
        rhi/shaders/scene.vert
        rhi/shaders/scene.frag
        rhi/shaders/line.vert
        rhi/shaders/line.frag
        rhi/shaders/beam.vert
        rhi/shaders/beam.frag
        rhi/shaders/beamvol.vert
        rhi/shaders/beamvol.frag
        rhi/shaders/surface.vert
        rhi/shaders/surface.frag
)



if(WIN32)
    find_library(ASSIMP_LIBRARY assimp-vc142-mt "${PROJECT_SOURCE_DIR}/libs")
    target_link_libraries(plugin-visualizer PRIVATE "${ASSIMP_LIBRARY}")
    include_directories("${PROJECT_SOURCE_DIR}/third-party")
else()
    find_package(assimp REQUIRED)
    target_link_libraries(plugin-visualizer PRIVATE assimp::assimp)
endif()

target_link_libraries(plugin-visualizer PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(plugin-visualizer PRIVATE Qt${QT_VERSION_MAJOR}::Gui)
target_link_libraries(plugin-visualizer PRIVATE Qt${QT_VERSION_MAJOR}::GuiPrivate)
target_link_libraries(plugin-visualizer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(plugin-visualizer PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia)

#target_compile_definitions(plugin-visualizer PRIVATE PLUGINVISUALIZER_LIBRARY)

target_link_libraries(plugin-visualizer PRIVATE photon-core)

set_target_properties(plugin-visualizer PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/visualizer/"
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/visualizer/"
                      LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/visualizer/"
                      LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/visualizer/"
                      OUTPUT_NAME "visualizer"
                      PREFIX ""
)

if(EXISTS "${PROJECT_SOURCE_DIR}/support-files")
    add_custom_command(TARGET plugin-visualizer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/support-files" "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/visualizer/"
    )
endif()

if(WIN32 AND EXISTS "${PROJECT_SOURCE_DIR}/dlls/windows")
    add_custom_command(TARGET plugin-visualizer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/dlls/windows" "${PROJECT_BINARY_DIR}/../photon-desktop/"
    )
endif()

# RhiRenderer::resolveModelPath() looks for <fixture-type>.<ext> files in
# <app>/models/ at runtime (not a Qt resource - these are loaded from real
# files, deduped/cached by path). Nothing deployed this folder before, so
# every fixture silently fell back to the placeholder box.
#
# rm -rf before copy_directory: resolveModelPath() prefers .fbx over .glb/etc
# by extension order, so a stale <type>.fbx left behind in an existing build
# directory after the source asset was renamed/removed (copy_directory only
# copies/overwrites - it never deletes) would silently keep shadowing a
# replacement in another format.
if(EXISTS "${PROJECT_SOURCE_DIR}/resources/model")
    add_custom_command(TARGET plugin-visualizer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${PROJECT_BINARY_DIR}/../photon-desktop/models/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/resources/model" "${PROJECT_BINARY_DIR}/../photon-desktop/models/"
    )
endif()

