add_executable(noko-desktop
  main.cc
  utils.cc
  homescreen.cc
  clay.c
  sdl_clay.c
)
if (EMSCRIPTEN)
  target_compile_options(noko-desktop PRIVATE -DEMSCRIPTEN)
  target_link_options(noko-desktop PRIVATE --shell-file "${CMAKE_CURRENT_SOURCE_DIR}/../webport/index.html" --preload-file "${CMAKE_CURRENT_SOURCE_DIR}/../public@/" -sINITIAL_MEMORY=70254592)
  set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
  set(SDLIMAGE_VENDORED ON)
  set(SDLTTF_VENDORED ON)
endif()

# Add a macro defining the projects font directory
# This will avoid errors for the time being since otherwise you would need to 
# be in the build directory every time you run noko-desktop
target_compile_definitions(noko-desktop PUBLIC PUBLIC_FOLDER=\"${CMAKE_SOURCE_DIR}/public/\")

# Try to find system-installed SDL3 components
find_package(SDL3 QUIET)
find_package(SDL3_image QUIET)
find_package(SDL3_ttf QUIET)

include(FetchContent)
if (NOT SDL3_FOUND)
message(STATUS "System SDL3 not found, fetching from source")
# Im pretty sure this compile flag is for sdl and not the others, correct me if im wrong - bear
set(SDL_LIBC ON)
set(SDL_TEST_LIBRARY OFF)
set(SDL_TESTS OFF)
FetchContent_Declare(
  sdl
  URL https://github.com/libsdl-org/SDL/releases/download/release-3.2.16/SDL3-3.2.16.tar.gz
)
FetchContent_MakeAvailable(sdl)
endif()

if (NOT SDL3_image_FOUND)
message(STATUS "System SDL3_image not found, fetching from source")
set(SDLIMAGE_AVIF OFF)	# disable formats we don't use to make the build faster and smaller.
set(SDLIMAGE_BMP OFF)
set(SDLIMAGE_JPEG OFF)
set(SDLIMAGE_WEBP OFF)
set(SDLIMAGE_BACKEND_IMAGEIO OFF)
set(SDLIMAGE_BACKEND_STB OFF)
set(SDLIMAGE_DEPS_SHARED OFF)
set(SDLIMAGE_RELOCATABLE ON)
set(SDLIMAGE_SAMPLES OFF)
set(SDLIMAGE_STRICT ON)
set(SDLIMAGE_VENDORED OFF)
set(SDLIMAGE_WEBP OFF)

FetchContent_Declare(
  sdl_image
  URL https://github.com/libsdl-org/SDL_image/releases/download/release-3.2.4/SDL3_image-3.2.4.tar.gz
)
FetchContent_MakeAvailable(sdl_image)
endif()

if (NOT SDL3_ttf_FOUND)
message(STATUS "System SDL3_ttf not found, fetching from source")
set(SDLTTF_VENDORED OFF)
set(SDLTTF_SAMPLES OFF)
FetchContent_Declare(
  sdl_ttf
  URL https://github.com/libsdl-org/SDL_ttf/releases/download/release-3.2.2/SDL3_ttf-3.2.2.tar.gz
)
FetchContent_MakeAvailable(sdl_ttf)
endif()

# Always download clay and libt9 since they are small & not packaged
set(CLAY_INCLUDE_ALL_EXAMPLES OFF)
FetchContent_Declare(
  clay
  URL https://github.com/nicbarker/clay/archive/refs/tags/v0.14.zip
)
FetchContent_MakeAvailable(clay)

set(BUILD_LIB ON)
FetchContent_Declare(
  libt9
  GIT_REPOSITORY https://github.com/FoxMoss/libt9
  GIT_TAG        main
  GIT_SHALLOW 1
)
FetchContent_MakeAvailable(libt9)

FetchContent_Declare(
  inipp 
  URL https://github.com/mcmtroffaes/inipp/archive/refs/tags/1.0.13.tar.gz
)
FetchContent_MakeAvailable(inipp)


target_link_libraries(noko-desktop PRIVATE SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf t9 inipp::inipp)
target_include_directories(noko-desktop PRIVATE ${clay_SOURCE_DIR})

add_custom_command(TARGET noko-desktop POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Add clangd to the src directory")


