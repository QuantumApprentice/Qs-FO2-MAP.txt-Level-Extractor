cmake_minimum_required(VERSION 3.25)
project(QMAP_txt_editor LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(EXE_NAME "QMAP_txt_editor")
add_executable(${EXE_NAME})
target_sources(
    ${EXE_NAME} PRIVATE
    src/main.cpp
    src/map_txt_parser.cpp
    src/map_txt_gui.cpp
    dependencies/IO_Platform/io_Platform.cpp
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
    dependencies/imgui-1.92.7/backends/imgui_impl_sdl3.h
    dependencies/imgui-1.92.7/backends/imgui_impl_sdl3.cpp
    dependencies/imgui-1.92.7/imgui.h
    dependencies/imgui-1.92.7/imgui.cpp
    dependencies/imgui-1.92.7/imgui_demo.cpp
    dependencies/imgui-1.92.7/imgui_draw.cpp
    dependencies/imgui-1.92.7/imgui_tables.cpp
    dependencies/imgui-1.92.7/imgui_widgets.cpp
)

# add_subdirectory(
#     dependencies/glfw-3.4/              SYSTEM
# )
add_subdirectory(
    dependencies/glad1/                 SYSTEM
)

# find_package(
#     SDL2 REQUIRED
#     CONFIG REQUIRED
#     COMPONENTS SDL2
# )
# find_package(
#     SDL2 REQUIRED
#     CONFIG COMPONENTS
#     SDL2main
# )
# add_subdirectory(
#     dependencies/SDL2-2.32.0/           SYSTEM
# )
# if (TARGET SDL2::SDL2main)
#     target_link_libraries(
#         ${EXE_NAME} PRIVATE
#         SDL2::SDL2main
#     )
# endif()

find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
target_include_directories(
    QMAP_txt_editor PRIVATE
    ${X11_INCLUDE_DIR}
)
# target_link_libraries(
#     QMAP_txt_editor PRIVATE
#     glfw ${X11_LIBRARIES}
# )



##apparently SDL2 doesn't have drag hover
# find_package(SDL2 REQUIRED)
add_subdirectory(
    dependencies/SDL3-3.4.2/
)

target_include_directories(
    ImGui PUBLIC
    dependencies/glfw-3.4/include/
    dependencies/imgui-1.92.7/
    dependencies/imgui-1.92.7/backends/
    dependencies/IO_Platform/
    # dependencies/SDL3-3.4.2/include/
    # dependencies/SDL2-2.32.0/include/
    # ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(
    ImGui PRIVATE
    SDL3::SDL3
)

target_link_libraries(
    ${EXE_NAME} PRIVATE
    ImGui
    glad
    SDL3::SDL3
    # ${SDL2_LIBRARIES}
    # SDL2::SDL2
    # glfw
    ${OPENGL_LIBRARIES}
    X11::X11
)

