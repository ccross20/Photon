cmake_minimum_required(VERSION 3.14)

project(plugin-effects LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

set(SOURCES
    generator/beatintegereffect.h
    generator/beatintegereffect.cpp
    generator/noiseeffect.h
    generator/noiseeffect.cpp
    generator/pulseeffect.h
    generator/pulseeffect.cpp
    generator/sawtootheffect.h
    generator/sawtootheffect.cpp
    generator/sineeffect.h
    generator/sineeffect.cpp
    mask/randommaskeffect.h
    mask/randommaskeffect.cpp
    mask/treemaskeffect.h
    mask/treemaskeffect.cpp
    modifier/easeeffect.h
    modifier/easeeffect.cpp
    modifier/smootheffect.h
    modifier/smootheffect.cpp
    time/loopeffect.h
    time/loopeffect.cpp
    time/stuttereffect.h
    time/stuttereffect.cpp
    plugineffects.cpp
    plugineffects.h
)

add_library(plugin-effects SHARED ${SOURCES})



target_link_libraries(plugin-effects PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(plugin-effects PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)


include_directories("${PROJECT_SOURCE_DIR}/third-party")



#target_compile_definitions(plugin-visualizer PRIVATE PLUGINVISUALIZER_LIBRARY)

target_link_libraries(plugin-effects PRIVATE photon-core)

set_target_properties(plugin-effects PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/channel-effects/"
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/channel-effects/"
                      OUTPUT_NAME "channel-effects"
)

add_custom_command(TARGET plugin-effects POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/support-files" "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/channel-effects/"
)
add_custom_command(TARGET plugin-effects POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/dlls/windows" "${PROJECT_BINARY_DIR}/../photon-desktop/"
)

