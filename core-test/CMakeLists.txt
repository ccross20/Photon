cmake_minimum_required(VERSION 3.5)

project(photon-core-test LANGUAGES CXX)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Test Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Test Widgets REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    photon-test.cpp
    bus/bustest.h
    bus/bustest.cpp
    fixture/fixturecapabilitytest.h
    fixture/fixturecapabilitytest.cpp
    fixture/fixturecollectiontest.h
    fixture/fixturecollectiontest.cpp
    sequence/sequencetest.h
    sequence/sequencetest.cpp
)

enable_testing(true)

add_executable(photon-core-test
    ${PROJECT_SOURCES})

add_test(NAME photon-core-test COMMAND test)

target_link_libraries(photon-core-test PRIVATE Qt${QT_VERSION_MAJOR}::Test)
target_link_libraries(photon-core-test PRIVATE photon-core)
target_link_libraries(photon-core-test PRIVATE keira)


target_link_libraries(photon-core-test PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(photon-core-test PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(photon-core-test PRIVATE Qt${QT_VERSION_MAJOR}::Gui)


