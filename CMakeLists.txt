cmake_minimum_required(VERSION 3.25)
project(QMAP_txt_editor LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(EXE_NAME "QMAP_txt_editor")
add_executable(${EXE_NAME})
target_sources(
    ${EXE_NAME} PRIVATE
    src/main.cpp
    src/map_txt_parser.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_compile_definitions(
    ${EXE_NAME}
    PUBLIC WINDOWS
)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
target_compile_definitions(
    ${EXE_NAME}
    PUBLIC LINUX
)
endif()

add_library(ImGui
    dependencies/imgui-1.92.7/backends/imgui_impl_opengl3.h
    dependencies/imgui-1.92.7/backends/imgui_impl_opengl3.cpp
    dependencies/imgui-1.92.7/backends/imgui_impl_glfw.h
    dependencies/imgui-1.92.7/backends/imgui_impl_glfw.cpp
    dependencies/imgui-1.92.7/imgui.h
    dependencies/imgui-1.92.7/imgui.cpp
    dependencies/imgui-1.92.7/imgui_demo.cpp
    dependencies/imgui-1.92.7/imgui_draw.cpp
    dependencies/imgui-1.92.7/imgui_tables.cpp
    dependencies/imgui-1.92.7/imgui_widgets.cpp
)

add_subdirectory(
    dependencies/glfw-3.4/              SYSTEM
)
add_subdirectory(
    dependencies/glad1/                 SYSTEM
)

target_include_directories(
    ImGui PUBLIC
    dependencies/glfw-3.4/include/
    dependencies/imgui-1.92.7/
    dependencies/imgui-1.92.7/backends/
    dependencies/IO_Platform
)

target_link_libraries(
    ${EXE_NAME} PRIVATE
    ImGui
    glad
    glfw
)

