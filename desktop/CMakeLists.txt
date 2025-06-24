add_executable(noko-desktop
  main.c
  utils.c
)
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

target_link_libraries(noko-desktop PRIVATE SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf)

add_custom_command(TARGET noko-desktop POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Add clangd to the src directory")


