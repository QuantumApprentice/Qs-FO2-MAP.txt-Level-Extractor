cmake_minimum_required(VERSION 3.25)

project(QMAP_txt_editor LANGUAGES C CXX)

set(EXE_NAME QMAP_txt_editor)
set(EXE_NAME QMAP_txt_editor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
include(cmake/Dependencies.cmake)

FetchContent_Declare(
  IO_Platform
  GIT_REPOSITORY https://github.com/QuantumApprentice/IO_platform.git
  GIT_TAG        bdbb96384cc438b15126b642845e5971a022dc10
  GIT_SHALLOW    FALSE
)

FetchContent_GetProperties(IO_Platform)

if(NOT io_platform_POPULATED)
  FetchContent_Populate(IO_Platform)
endif()

message(STATUS "IO_Platform source dir: ${io_platform_SOURCE_DIR}")

set(APP_SOURCES
  src/main.cpp
  src/map_txt_parser.cpp
  src/map_txt_gui.cpp
  ${io_platform_SOURCE_DIR}/io_Platform.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND APP_SOURCES
    src/X11_drag_drop.cpp
  )
endif()

add_executable(${EXE_NAME}
  ${APP_SOURCES}
)

target_include_directories(${EXE_NAME}
  PRIVATE
    ${io_platform_SOURCE_DIR}
)

target_compile_definitions(${EXE_NAME}
  PRIVATE
    $<$<PLATFORM_ID:Windows>:WINDOWS>
    $<$<PLATFORM_ID:Linux>:LINUX>
)

find_package(OpenGL REQUIRED)

target_link_libraries(${EXE_NAME}
  PRIVATE
    ImGui
    glad
    SDL3::SDL3
    OpenGL::GL
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  find_package(X11 REQUIRED)

  target_link_libraries(${EXE_NAME}
    PRIVATE
      X11::X11
  )
endif()