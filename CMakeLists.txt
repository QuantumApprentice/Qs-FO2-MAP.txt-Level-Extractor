cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 17)
set(EXE_NAME QMAP_txt_extractor)
project(${EXE_NAME} LANGUAGES CXX)


if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_executable(${EXE_NAME} WIN32)   #makes program open without dos window
    set_target_properties(
        ${EXE_NAME} PROPERTIES
        LINK_FLAGS "/ENTRY:mainCRTStartup"
    )
    target_compile_definitions(
        ${EXE_NAME}
        PUBLIC WINDOWS
    )

elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(${EXE_NAME})
    target_compile_definitions(
        ${EXE_NAME}
        PUBLIC LINUX
    )
endif()


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

add_library(ImGui
    ${DEPS}/imgui-1.92.7/backends/imgui_impl_opengl3.h
    ${DEPS}/imgui-1.92.7/backends/imgui_impl_opengl3.cpp
    ${DEPS}/imgui-1.92.7/backends/imgui_impl_sdl3.h
    ${DEPS}/imgui-1.92.7/backends/imgui_impl_sdl3.cpp
    ${DEPS}/imgui-1.92.7/imgui.h
    ${DEPS}/imgui-1.92.7/imgui.cpp
    #${DEPS}/imgui-1.92.7/imgui_demo.cpp         #TODO: remove this for final compile
    ${DEPS}/imgui-1.92.7/imgui_draw.cpp
    ${DEPS}/imgui-1.92.7/imgui_tables.cpp
    ${DEPS}/imgui-1.92.7/imgui_widgets.cpp
)

add_subdirectory(${DEPS}/glad1/)

target_include_directories(
    ImGui PUBLIC
    ${DEPS}/glfw-3.4/include/
    ${DEPS}/imgui-1.92.7/
    ${DEPS}/imgui-1.92.7/backends/
    ${DEPS}/IO_Platform/
)

set(SDL_STATIC ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    # apparently SDL2 doesn't have drag hover
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        683181b47cfabd293e3ea409f838915b8297a4fd
    GIT_SHALLOW    FALSE
)
FetchContent_MakeAvailable(SDL3)

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
    SDL3::SDL3-static
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
