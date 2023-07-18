cmake_minimum_required(VERSION 3.14)

project(plugin-output-nodes LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 REQUIRED COMPONENTS Network)

set(SOURCES
    third-party/sacn/consts.h
    third-party/sacn/e1_11.h
    third-party/sacn/firewallcheck.h
    third-party/sacn/firewallcheck.cpp
    third-party/sacn/fpscounter.h
    third-party/sacn/fpscounter.cpp
    third-party/sacn/sacndiscoveredsourcelistmodel.h
    third-party/sacn/sacndiscoveredsourcelistmodel.cpp
    third-party/sacn/sacndiscovery.h
    third-party/sacn/sacndiscovery.cpp
    third-party/sacn/sacnlistener.h
    third-party/sacn/sacnlistener.cpp
    third-party/sacn/sacnsender.h
    third-party/sacn/sacnsender.cpp
    third-party/sacn/sacnsocket.h
    third-party/sacn/sacnsocket.cpp
    third-party/sacn/sacnuniverselistmodel.h
    third-party/sacn/sacnuniverselistmodel.cpp
    third-party/sacn/streamcommon.h
    third-party/sacn/streamcommon.cpp
    third-party/sacn/streamingacn.h
    third-party/sacn/streamingacn.cpp
    third-party/sacn/ACNShare/CID.h
    third-party/sacn/ACNShare/CID.cpp
    third-party/sacn/ACNShare/defpack.h
    third-party/sacn/ACNShare/deftypes.h
    third-party/sacn/ACNShare/ipaddr.h
    third-party/sacn/ACNShare/ipaddr.cpp
    third-party/sacn/ACNShare/tock.h
    third-party/sacn/ACNShare/tock.cpp
    third-party/sacn/ACNShare/VHD.h
    third-party/sacn/ACNShare/VHD.cpp
    pluginoutputnodes.h
    pluginoutputnodes.cpp
    pluginoutputnodes.h
    sacntransmitnode.h
    sacntransmitnode.cpp
)

add_library(plugin-output-nodes SHARED ${SOURCES})


#add_library( EtcPal SHARED IMPORTED )
#set_target_properties( EtcPal PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/libs/EtcPal.lib )
#add_library( sACN SHARED IMPORTED )
#set_target_properties( sACN PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/libs/sACN.lib )

#find_library(EtcPal ${PROJECT_SOURCE_DIR}/libs)
#target_link_libraries(plugin-output-nodes PRIVATE EtcPal)


target_include_directories(plugin-output-nodes PUBLIC ../keira)
target_link_libraries(plugin-output-nodes PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(plugin-output-nodes PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)
target_link_libraries(plugin-output-nodes PRIVATE Qt${QT_VERSION_MAJOR}::Network)


include_directories("${PROJECT_SOURCE_DIR}/third-party/sacn")
include_directories("${PROJECT_SOURCE_DIR}/third-party/sacn/ACNShare")



#target_compile_definitions(plugin-visualizer PRIVATE PLUGINVISUALIZER_LIBRARY)

target_link_libraries(plugin-output-nodes PRIVATE keira)
target_link_libraries(plugin-output-nodes PRIVATE photon-core)

set_target_properties(plugin-output-nodes PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/output-nodes/"
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/output-nodes/"
                      OUTPUT_NAME "output-nodes"
)

add_custom_command(TARGET plugin-output-nodes POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/support-files" "${PROJECT_BINARY_DIR}/../photon-desktop/extensions/output-nodes/"
)
add_custom_command(TARGET plugin-output-nodes POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/dlls/windows" "${PROJECT_BINARY_DIR}/../photon-desktop/"
)
