cmake_minimum_required(VERSION 3.5)

project(rhi-spike LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui GuiPrivate Widgets)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Console subsystem so qInfo/qWarning reach stdout/stderr (a WIN32 GUI app
# detaches from the console and routes Qt logging to the debugger instead).
add_executable(rhi-spike main.cpp)
if (WIN32)
    set_target_properties(rhi-spike PROPERTIES WIN32_EXECUTABLE OFF)
endif()

target_link_libraries(rhi-spike PRIVATE photon-core keira)
target_link_libraries(rhi-spike PRIVATE Qt6::Core Qt6::Gui Qt6::GuiPrivate Qt6::Widgets)
