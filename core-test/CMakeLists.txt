cmake_minimum_required(VERSION 3.5)

project(photon-core-test LANGUAGES CXX)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Test Widgets Sql REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Test Widgets Sql REQUIRED)

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
    fixture/colorcalibrationtest.h
    fixture/colorcalibrationtest.cpp
    fixture/lasertest.h
    fixture/lasertest.cpp
    virtualdj/virtualdjplaybacktest.h
    virtualdj/virtualdjplaybacktest.cpp
    fixture/fixturecollectiontest.h
    fixture/fixturecollectiontest.cpp
    sequence/sequencetest.h
    sequence/sequencetest.cpp
    scene/sceneobjecttest.h
    scene/sceneobjecttest.cpp
    scene/fallofftest.h
    scene/fallofftest.cpp
    project/surfaceownershiptest.h
    project/surfaceownershiptest.cpp
    project/projectresourcetest.h
    project/projectresourcetest.cpp
    project/projectmodeltest.h
    project/projectmodeltest.cpp
    project/tagtest.h
    project/tagtest.cpp
    gui/propertyaddresstest.h
    gui/propertyaddresstest.cpp
    library/songlibrarytest.h
    library/songlibrarytest.cpp
)

enable_testing(true)

add_executable(photon-core-test
    ${PROJECT_SOURCES})

add_test(NAME photon-core-test COMMAND test)

target_link_libraries(photon-core-test PRIVATE Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Sql)
target_link_libraries(photon-core-test PRIVATE photon-core)
target_link_libraries(photon-core-test PRIVATE keira)
target_compile_definitions(photon-core-test PRIVATE PHOTON_SOURCE_DIR="${CMAKE_SOURCE_DIR}")


target_link_libraries(photon-core-test PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(photon-core-test PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(photon-core-test PRIVATE Qt${QT_VERSION_MAJOR}::Gui)


