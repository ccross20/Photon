cmake_minimum_required(VERSION 3.14)

project(plugin-visualizer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS 3DInput Core Gui Qml Quick QuickWidgets Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS 3DInput Core Gui Qml Quick QuickWidgets Widgets)

set(SOURCES
    plugin-visualizer-global.h
    pluginvisualizer.cpp
    pluginvisualizer.h
    visualizerpanel.h
    visualizerpanel.cpp
)

qt_add_resources(SOURCES resources.qrc)
add_library(plugin-visualizer SHARED ${SOURCES})



find_library(ASSIMP_LIBRARY assimp-vc142-mt "${PROJECT_SOURCE_DIR}/libs")
target_link_libraries(plugin-visualizer PRIVATE "${ASSIMP_LIBRARY}")

target_link_libraries(plugin-visualizer PUBLIC
    Qt::3DInput
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::QuickWidgets
    Qt::Widgets
)


include_directories("${PROJECT_SOURCE_DIR}/third-party")

set_source_files_properties("./resources/qml/AnimatedEntity.qml"
    PROPERTIES QT_RESOURCE_ALIAS "AnimatedEntity.qml"
)

set_source_files_properties("./resources/qml/main.qml"
    PROPERTIES QT_RESOURCE_ALIAS "main.qml"
)

set(plugin-visualizer_resource_files
    "./resources/qml/AnimatedEntity.qml"
    "./resources/qml/main.qml"
)

qt6_add_resources(plugin-visualizer "plugin-visualizer"
    PREFIX
        "/"
    FILES
        ${plugin-visualizer_resource_files}
)



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

