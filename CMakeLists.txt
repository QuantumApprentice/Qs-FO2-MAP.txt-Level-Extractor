cmake_minimum_required(VERSION 3.30)
project(QMAP_txt_editor LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(EXE_NAME QMAP_txt_editor)
add_executable(${EXE_NAME})

include(FetchContent)
set(DEPS "dependencies")
option(USE_TARBALL "Use extracted tarball vendored dependencies instead of git submodules" OFF)

if(USE_TARBALL)
    set(DEPS "vendor")
    # apparently SDL2 doesn't have drag hover
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/vendor/SDL3/CMakeLists.txt")
        message(SEND_ERROR "Unable to find ${CMAKE_SOURCE_DIR}/vendor/SDL3/ - Please extract from tarball")
    endif()
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/vendor/IO_Platform/io_Platform.cpp")
        message(SEND_ERROR "Unable to find ${CMAKE_SOURCE_DIR}/vendor/IO_platform/ - Please extract from tarball")
    endif()
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/vendor/imgui/imgui.cpp")
        message(SEND_ERROR "Unable to find ${CMAKE_SOURCE_DIR}/vendor/imgui/ - Please extract from tarball")
    endif()
endif()


target_sources(
    ${EXE_NAME} PRIVATE
    src/main.cpp
    src/map_txt_parser.cpp
    src/map_txt_gui.cpp
)
target_sources(
    ${EXE_NAME} PRIVATE
    ${DEPS}/IO_Platform/io_Platform.cpp
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
    ${DEPS}/imgui-1.92.7/backends/imgui_impl_opengl3.h
    ${DEPS}/imgui-1.92.7/backends/imgui_impl_opengl3.cpp
    ${DEPS}/imgui-1.92.7/backends/imgui_impl_sdl3.h
    ${DEPS}/imgui-1.92.7/backends/imgui_impl_sdl3.cpp
    ${DEPS}/imgui-1.92.7/imgui.h
    ${DEPS}/imgui-1.92.7/imgui.cpp
    ${DEPS}/imgui-1.92.7/imgui_demo.cpp         #TODO: remove this for final compile
    ${DEPS}/imgui-1.92.7/imgui_draw.cpp
    ${DEPS}/imgui-1.92.7/imgui_tables.cpp
    ${DEPS}/imgui-1.92.7/imgui_widgets.cpp
)

add_subdirectory(${DEPS}/glad1/)
add_subdirectory(${DEPS}/SDL3-3.4.2/)

target_include_directories(
    ImGui PUBLIC
    ${DEPS}/glfw-3.4/include/
    ${DEPS}/imgui-1.92.7/
    ${DEPS}/imgui-1.92.7/backends/
    ${DEPS}/IO_Platform/
)

# ImGui needs its own SDL3 links
target_link_libraries(
    ImGui PRIVATE
    SDL3::SDL3
)
find_package(OpenGL REQUIRED)
target_link_libraries(
    ${EXE_NAME} PRIVATE
    ImGui
    glad
    SDL3::SDL3
    OpenGL::GL
)


set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-source")

set(CPACK_SOURCE_IGNORE_FILES
  "/\\.git/"
  "/_build/"
  "/build/"
  "/build-.*/"
  "/out/"
  "/\\.vs/"
  "/\\.vscode/"
  "/vendor-source-package/"
  "/CMakeFiles/"
  "/CMakeCache\\.txt$"
  "/\\.ninja_log$"
  "/\\.ninja_deps$"
  "/_CPack_Packages/"
  "/CPackConfig\\.cmake$"
  "/CPackSourceConfig\\.cmake$"
)

include(CPack)
