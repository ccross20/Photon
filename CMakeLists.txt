cmake_minimum_required(VERSION 3.5)

project(photon LANGUAGES CXX)

add_subdirectory(core)
add_subdirectory(core-test)
add_subdirectory(plugin-effects)
add_subdirectory(plugin-clip-effects)
add_subdirectory(plugin-falloff)
add_subdirectory(plugin-nodes)
add_subdirectory(plugin-output-nodes)
add_subdirectory(plugin-vamp)
add_subdirectory(plugin-visualizer)
add_subdirectory(keira)
add_subdirectory(keira-playground)
add_subdirectory(keira-test)
add_subdirectory(photon-desktop)

enable_testing()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


