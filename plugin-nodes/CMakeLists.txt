cmake_minimum_required(VERSION 3.14)

project(plugin-nodes LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

set(SOURCES
    plugin-nodes-global.h
    pluginnodes.cpp
    pluginnodes.h
    fixture/setfixturepan.h
    fixture/setfixturepan.cpp
    fixture/setfixturetilt.h
    fixture/setfixturetilt.cpp
)

add_library(plugin-nodes SHARED ${SOURCES})

target_include_directories(plugin-nodes PUBLIC ../keira)
target_link_libraries(plugin-nodes PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(plugin-nodes PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

#target_compile_definitions(plugin-visualizer PRIVATE PLUGINVISUALIZER_LIBRARY)
target_link_libraries(plugin-nodes PRIVATE keira)
target_link_libraries(plugin-nodes PRIVATE photon-core)

set_target_properties(plugin-nodes PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/nodes/"
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/nodes/"
                      OUTPUT_NAME "nodes"
)

add_custom_command(TARGET plugin-nodes POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/support-files" "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/nodes/"
)

