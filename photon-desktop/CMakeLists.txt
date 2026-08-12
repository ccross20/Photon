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

# keira/photon-core are SHARED libs; photon-desktop loads them from its own
# directory at runtime, not from their build directories, so a stale copy
# here silently shadows a freshly-built DLL until this re-copies it.
#
# This can't be a POST_BUILD step on photon-desktop itself: MSVC/ninja treat
# the exe as up to date whenever its import .lib is unchanged, which happens
# whenever a photon-core/keira change doesn't alter exported symbols (e.g. an
# edit to a private, non-exported class) - so the exe wouldn't relink and a
# POST_BUILD step attached to it would silently never run, leaving a stale
# DLL next to the exe. A separate custom target keyed on the DLLs'
# actual file paths reruns whenever their content changes, independent of
# whether the exe itself needs relinking.
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/deploy-runtime-dlls.stamp
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:keira>
        $<TARGET_FILE:photon-core>
        ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/deploy-runtime-dlls.stamp
    DEPENDS $<TARGET_FILE:keira> $<TARGET_FILE:photon-core>
    COMMENT "Copying keira/photon-core DLLs next to photon-desktop.exe"
)
add_custom_target(photon-desktop-deploy-dlls ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/deploy-runtime-dlls.stamp
)
add_dependencies(photon-desktop photon-desktop-deploy-dlls)


target_link_libraries(photon-desktop PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(photon-desktop PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(photon-desktop PRIVATE Qt${QT_VERSION_MAJOR}::Gui)


