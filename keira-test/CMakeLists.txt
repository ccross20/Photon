cmake_minimum_required(VERSION 3.5)

project(keira-test LANGUAGES CXX)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Test Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Test Widgets REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    keira-test.cpp
    evaluationtest.h
    evaluationtest.cpp
)

enable_testing(true)

add_executable(keira-test
    ${PROJECT_SOURCES})

add_test(NAME keira-test COMMAND test)

target_link_libraries(keira-test PRIVATE Qt${QT_VERSION_MAJOR}::Test)
target_link_libraries(keira-test PRIVATE keira)


target_link_libraries(keira-test PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(keira-test PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(keira-test PRIVATE Qt${QT_VERSION_MAJOR}::Gui)


