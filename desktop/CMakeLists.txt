add_executable(noko-desktop
  main.cc
  utils.cc
  homescreen.cc
  clay.c
  sdl_clay.c
)
if (EMSCRIPTEN)
  target_compile_options(noko-desktop PRIVATE -DEMSCRIPTEN)
  target_link_options(noko-desktop PRIVATE --shell-file "${CMAKE_CURRENT_SOURCE_DIR}/../webport/index.html" --preload-file "${CMAKE_CURRENT_SOURCE_DIR}/../public@/")
  set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
  set(SDLTTF_VENDORED ON)
  set(SDLIMAGE_VENDORED ON)
  set(SDLIMAGE_AVIF OFF)	# disable formats we don't use to make the build faster and smaller.
  set(SDLIMAGE_BMP OFF)
  set(SDLIMAGE_JPEG OFF)
  set(SDLIMAGE_WEBP OFF)
endif()

include(FetchContent)
FetchContent_Declare(
  sdl
  GIT_REPOSITORY https://github.com/libsdl-org/SDL/
  GIT_TAG        038a3806eb09432b6dd31b3c884ac236e44bb936
)
FetchContent_MakeAvailable(sdl)

set(SDLIMAGE_WEBP OFF)
FetchContent_Declare(
  sdl_image
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image
  GIT_TAG        925c19db4d0bc9809fd3ac25c7e0ef771b668390
)
FetchContent_MakeAvailable(sdl_image)

FetchContent_Declare(
  sdl_ttf
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf
  GIT_TAG        ffa54ca2ec6b89b4f6dc92ab113484cfa2ccc3a7
)
FetchContent_MakeAvailable(sdl_ttf)

set(CLAY_INCLUDE_ALL_EXAMPLES OFF)
FetchContent_Declare(
  clay
  GIT_REPOSITORY https://github.com/nicbarker/clay
  GIT_TAG        281f961e3d7272e5627a7618aedddd70faf04668
)
FetchContent_MakeAvailable(clay)

set(BUILD_LIB ON)
FetchContent_Declare(
  libt9
  GIT_REPOSITORY https://github.com/FoxMoss/libt9
  GIT_TAG        7307f13d2b0e1c1a33841da75926743019f3bbf6
)
FetchContent_MakeAvailable(libt9)

target_link_libraries(noko-desktop PRIVATE SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf t9)
target_include_directories(noko-desktop PRIVATE ${clay_SOURCE_DIR})

add_custom_command(TARGET noko-desktop POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Add clangd to the src directory")


