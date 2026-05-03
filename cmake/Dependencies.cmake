include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG        683181b47cfabd293e3ea409f838915b8297a4fd
  GIT_SHALLOW    FALSE
)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        dac07199cfd761113d966eb8ad739254e10df2fe
  GIT_SHALLOW    FALSE
)

add_library(glad STATIC
  ${CMAKE_SOURCE_DIR}/dependencies/glad1/src/glad.c
)

target_include_directories(glad
  PUBLIC
    ${CMAKE_SOURCE_DIR}/dependencies/glad1/include
)

FetchContent_MakeAvailable(SDL3 )

FetchContent_GetProperties(imgui)

if(NOT imgui_POPULATED)
  FetchContent_Populate(imgui)

  add_library(ImGui STATIC)

  target_sources(ImGui
    PRIVATE
      ${imgui_SOURCE_DIR}/imgui.cpp
      ${imgui_SOURCE_DIR}/imgui_draw.cpp
      ${imgui_SOURCE_DIR}/imgui_tables.cpp
      ${imgui_SOURCE_DIR}/imgui_widgets.cpp
      ${imgui_SOURCE_DIR}/imgui_demo.cpp
      ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
      ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  )

  target_include_directories(ImGui
    PUBLIC
      ${imgui_SOURCE_DIR}
      ${imgui_SOURCE_DIR}/backends
  )

  target_link_libraries(ImGui
    PUBLIC
      SDL3::SDL3
  )
endif()
