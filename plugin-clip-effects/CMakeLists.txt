cmake_minimum_required(VERSION 3.14)

project(plugin-clip-effects LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)


set(SOURCES
    pluginclipeffects.cpp
    pluginclipeffects.h
    canvas/draw/drawrectangleclipeffect.h
    canvas/draw/drawrectangleclipeffect.cpp
    pan-and-tilt/tracepathclip.h
    pan-and-tilt/tracepathclip.cpp
)



add_library(plugin-clip-effects SHARED ${SOURCES})



target_link_libraries(plugin-clip-effects PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(plugin-clip-effects PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)


include_directories("${PROJECT_SOURCE_DIR}/third-party")



#target_compile_definitions(plugin-visualizer PRIVATE PLUGINVISUALIZER_LIBRARY)

target_link_libraries(plugin-clip-effects PRIVATE photon-core)

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
