cmake_minimum_required(VERSION 3.14)

project(plugin-visualizer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets OpenGLWidgets)

set(SOURCES
    component/abstractcomponent.h
    component/abstractcomponent.cpp
    component/transformcomponent.h
    component/transformcomponent.cpp
    component/abstractmesh.h
    component/abstractmesh.cpp
    component/abstractlight.h
    component/abstractlight.cpp
    component/cameracomponent.h
    component/cameracomponent.cpp
    component/cubemesh.h
    component/cubemesh.cpp
    component/infinitelight.h
    component/infinitelight.cpp
    component/planemesh.h
    component/planemesh.cpp
    component/materialcomponent.h
    component/materialcomponent.cpp
    component/material/visiblelightmaterial.h
    component/material/visiblelightmaterial.cpp
    component/spotlight.h
    component/spotlight.cpp
    camera.h
    camera.cpp
    freecamera.h
    freecamera.cpp
    orbitcamera.h
    orbitcamera.cpp
    entity.h
    entity.cpp
    fixturemodel.h
    fixturemodel.cpp
    geometry/Line.h
    geometry/Line.cpp
    geometry/Matrices.h
    geometry/Matrices.cpp
    geometry/Pipe.h
    geometry/Pipe.cpp
    geometry/Plane.h
    geometry/Plane.cpp
    geometry/Vectors.h
    geometry/trussgeometry.h
    geometry/trussgeometry.cpp
    modelfactory.h
    modelfactory.cpp
    modelloader.h
    modelloader.cpp
    modifier/modelmodifier.h
    modifier/modelmodifier.cpp
    modifier/lampmodifier.h
    modifier/lampmodifier.cpp
    modifier/panmodelmodifier.h
    modifier/panmodelmodifier.cpp
    modifier/tiltmodelmodifier.h
    modifier/tiltmodelmodifier.cpp
    openglviewport.h
    openglviewport.cpp
    plugin-visualizer-global.h
    pluginvisualizer.cpp
    pluginvisualizer.h
    scene.h
    scene.cpp
    sceneobjectmodel.h
    sceneobjectmodel.cpp
    texture.h
    texture.cpp
    trussmodel.h
    trussmodel.cpp
    visualizerpanel.h
    visualizerpanel.cpp
)

qt_add_resources(SOURCES resources.qrc)
add_library(plugin-visualizer SHARED ${SOURCES})



find_library(ASSIMP_LIBRARY assimp-vc142-mt "${PROJECT_SOURCE_DIR}/libs")
target_link_libraries(plugin-visualizer PRIVATE "${ASSIMP_LIBRARY}")

target_link_libraries(plugin-visualizer PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(plugin-visualizer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(plugin-visualizer PRIVATE Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
target_link_libraries(plugin-visualizer PRIVATE Qt${QT_VERSION_MAJOR}::OpenGL)
target_link_libraries(plugin-visualizer PRIVATE ${OPENGL_LIBRARIES} Opengl32)


include_directories("${PROJECT_SOURCE_DIR}/third-party")

#target_compile_definitions(plugin-visualizer PRIVATE PLUGINVISUALIZER_LIBRARY)

target_link_libraries(plugin-visualizer PRIVATE photon-core)

set_target_properties(plugin-visualizer PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/visualizer/"
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/visualizer/"
                      OUTPUT_NAME "visualizer"
)

add_custom_command(TARGET plugin-visualizer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/support-files" "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/visualizer/"
)

add_custom_command(TARGET plugin-visualizer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/dlls/windows" "${PROJECT_BINARY_DIR}/../photon-desktop/"
)

