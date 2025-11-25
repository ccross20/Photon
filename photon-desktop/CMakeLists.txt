cmake_minimum_required(VERSION 3.5)

project(photon-desktop LANGUAGES CXX)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    desktop.cpp
)

enable_testing(true)

add_executable(photon-desktop
    ${PROJECT_SOURCES})
if (WIN32)
    set_target_properties(photon-desktop PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

target_link_libraries(photon-desktop PRIVATE keira)
target_link_libraries(photon-desktop PRIVATE photon-core)


target_link_libraries(photon-desktop PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(photon-desktop PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(photon-desktop PRIVATE Qt${QT_VERSION_MAJOR}::Gui)


