cmake_minimum_required(VERSION 3.14)

project(plugin-clip-effects LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets Concurrent OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Concurrent OpenGLWidgets)


set(SOURCES
    pluginclipeffects.cpp
    pluginclipeffects.h
    canvas/fill/fillnoiseeffect.h
    canvas/fill/fillnoiseeffect.cpp
    canvas/draw/drawellipseclipeffect.h
    canvas/draw/drawellipseclipeffect.cpp
    canvas/draw/drawrectangleclipeffect.h
    canvas/draw/drawrectangleclipeffect.cpp
    pan-and-tilt/tracepathclip.h
    pan-and-tilt/tracepathclip.cpp
    strobe/beatstrobeeffect.h
    strobe/beatstrobeeffect.cpp
    util/gilconcurrent.h
)



add_library(plugin-clip-effects SHARED ${SOURCES}
    canvas/fill/fillcoloreffect.h canvas/fill/fillcoloreffect.cpp
    pan-and-tilt/aligntoarrowclip.h pan-and-tilt/aligntoarrowclip.cpp
    clip-effect-resources.qrc
    canvas/blurclipeffect.h canvas/blurclipeffect.cpp
    canvas/distort/kaleidoscopeclipeffect.h canvas/distort/kaleidoscopeclipeffect.cpp
    canvas/draw/cloudtunnel.h canvas/draw/cloudtunnel.cpp
    canvas/draw/imageclipeffect.h canvas/draw/imageclipeffect.cpp
    canvas/style/strokeclipeffect.h canvas/style/strokeclipeffect.cpp)



target_link_libraries(plugin-clip-effects PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(plugin-clip-effects PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
)


include_directories("${PROJECT_SOURCE_DIR}/third-party")
include_directories("${PROJECT_SOURCE_DIR}/third-party/boost")



#target_compile_definitions(plugin-visualizer PRIVATE PLUGINVISUALIZER_LIBRARY)

target_link_libraries(plugin-clip-effects PRIVATE photon-core)
target_link_libraries(plugin-clip-effects PRIVATE
    Qt6::OpenGLWidgets
    Qt6::OpenGL
)

set_target_properties(plugin-clip-effects PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/clip-effects/"
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/clip-effects/"
                      OUTPUT_NAME "clip-effects"
)

add_custom_command(TARGET plugin-clip-effects POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/support-files" "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/clip-effects/"
)
add_custom_command(TARGET plugin-clip-effects POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/dlls/windows" "${PROJECT_BINARY_DIR}/../photon-desktop/"
)
