cmake_minimum_required(VERSION 3.5)

project(keira VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)


set(PROJECT_SOURCES
        model/parameter/buttonparameter.h
        model/parameter/buttonparameter.cpp
        model/parameter/decimalparameter.h
        model/parameter/decimalparameter.cpp
        model/parameter/integerparameter.h
        model/parameter/integerparameter.cpp
        model/parameter/optionparameter.h
        model/parameter/optionparameter.cpp
        model/parameter/parameter.h
        model/parameter/parameter_p.h
        model/parameter/parameter.cpp
        model/parameter/stringparameter.h
        model/parameter/stringparameter.cpp
        keira-global.h
        library/nodelibrary.h
        library/nodelibrary.cpp
        model/graph.h
        model/graph_p.h
        model/graph.cpp
        model/graphevaluator.h
        model/graphevaluator.cpp
        model/graphsorter.h
        model/graphsorter.cpp
        model/node.h
        model/node_p.h
        model/node.cpp
        view/nodeitem.h
        view/nodeitem.cpp
        view/portitem.h
        view/portitem.cpp
        view/scene.h
        view/scene.cpp
        view/viewer.h
        view/viewer.cpp
        view/wireitem.h
        view/wireitem.cpp
)

add_library(keira SHARED ${PROJECT_SOURCES})

target_include_directories(keira PUBLIC ../keira)

target_link_libraries(keira PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)


target_compile_definitions(keira PRIVATE KEIRA_LIBRARY)


