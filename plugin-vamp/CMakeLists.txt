cmake_minimum_required(VERSION 3.14)

project(plugin-vamp LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 REQUIRED COMPONENTS Network MultimediaWidgets)

set(SOURCES
    third-party/vamp-hostsdk/acsymbols.c
    third-party/vamp-hostsdk/Files.cpp
    third-party/vamp-hostsdk/Files.h
    third-party/vamp-hostsdk/host-c.cpp
    third-party/vamp-hostsdk/host-c.h
    third-party/vamp-hostsdk/hostguard.h
    third-party/vamp-hostsdk/Plugin.h
    third-party/vamp-hostsdk/PluginBase.h
    third-party/vamp-hostsdk/PluginBufferingAdapter.cpp
    third-party/vamp-hostsdk/PluginBufferingAdapter.h
    third-party/vamp-hostsdk/PluginChannelAdapter.cpp
    third-party/vamp-hostsdk/PluginChannelAdapter.h
    third-party/vamp-hostsdk/PluginHostAdapter.cpp
    third-party/vamp-hostsdk/PluginHostAdapter.h
    third-party/vamp-hostsdk/PluginInputDomainAdapter.cpp
    third-party/vamp-hostsdk/PluginInputDomainAdapter.h
    third-party/vamp-hostsdk/PluginLoader.cpp
    third-party/vamp-hostsdk/PluginLoader.h
    third-party/vamp-hostsdk/PluginSummarisingAdapter.cpp
    third-party/vamp-hostsdk/PluginSummarisingAdapter.h
    third-party/vamp-hostsdk/PluginWrapper.cpp
    third-party/vamp-hostsdk/PluginWrapper.h
    third-party/vamp-hostsdk/RealTime.cpp
    third-party/vamp-hostsdk/RealTime.h
    third-party/vamp-hostsdk/vamp-hostsdk.h
    third-party/vamp-hostsdk/Window.h
    beatdetectionprocess.h
    beatdetectionprocess.cpp
    vamp.h
    vamp.cpp
    pluginvamp.cpp
    pluginvamp.h
)

add_library(plugin-vamp SHARED ${SOURCES})


#add_library( EtcPal SHARED IMPORTED )
#set_target_properties( EtcPal PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/libs/EtcPal.lib )
#add_library( sACN SHARED IMPORTED )
#set_target_properties( sACN PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/libs/sACN.lib )

#find_library(EtcPal ${PROJECT_SOURCE_DIR}/libs)
#target_link_libraries(plugin-vamp PRIVATE EtcPal)


target_include_directories(plugin-vamp PUBLIC ../keira)
target_link_libraries(plugin-vamp PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(plugin-vamp PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)
target_link_libraries(plugin-vamp PRIVATE Qt${QT_VERSION_MAJOR}::Network Qt6::MultimediaWidgets)


include_directories("${PROJECT_SOURCE_DIR}/third-party/")
include_directories("${PROJECT_SOURCE_DIR}/third-party/vamp-hostsdk")
include_directories("${PROJECT_SOURCE_DIR}/third-party/vamp-sdk")
include_directories("${PROJECT_SOURCE_DIR}/third-party/vamp")



#target_compile_definitions(plugin-visualizer PRIVATE PLUGINVISUALIZER_LIBRARY)

target_link_libraries(plugin-vamp PRIVATE keira)
target_link_libraries(plugin-vamp PRIVATE photon-core)

set_target_properties(plugin-vamp PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/vamp/"
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/vamp/"
                      OUTPUT_NAME "vamp"
)

add_custom_command(TARGET plugin-vamp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/support-files" "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/vamp/"
)
add_custom_command(TARGET plugin-vamp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/dlls/windows" "${PROJECT_BINARY_DIR}/../photon-desktop/"
)
