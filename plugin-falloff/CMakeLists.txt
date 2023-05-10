cmake_minimum_required(VERSION 3.14)

project(plugin-falloff LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

set(SOURCES
    mapped2dfalloff.h
    mapped2dfalloff.cpp
    randomfalloff.h
    randomfalloff.cpp
    pluginfalloff.cpp
    pluginfalloff.h
)

add_library(plugin-falloff SHARED ${SOURCES})

target_link_libraries(plugin-falloff PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(plugin-falloff PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(plugin-falloff PRIVATE photon-core)

set_target_properties(plugin-falloff PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/falloff-effects/"
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/falloff-effects/"
                      OUTPUT_NAME "falloff-effects"
)

add_custom_command(TARGET plugin-falloff POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/support-files" "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/falloff-effects/"
)

