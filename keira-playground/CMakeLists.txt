cmake_minimum_required(VERSION 3.5)

project(keira-playground LANGUAGES CXX)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    main.cpp
    playgroundwidget.h
    playgroundwidget.cpp
)


add_executable(keira-playground
    ${PROJECT_SOURCES})


target_link_libraries(keira-playground PUBLIC keira)


target_link_libraries(keira-playground PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(keira-playground PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(keira-playground PRIVATE Qt${QT_VERSION_MAJOR}::Gui)

target_include_directories(keira-playground PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/../keira"
                          )


