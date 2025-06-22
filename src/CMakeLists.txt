add_executable(noko
  main.c
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(wlroots REQUIRED wlroots)
pkg_check_modules(wayland REQUIRED wayland-server)
pkg_check_modules(pixman REQUIRED pixman-1)
target_link_libraries(noko ${wlroots_LIBRARIES} ${wayland_LIBRARIES} ${pixman_LIBRARIES})
target_include_directories(noko PRIVATE ${wlroots_INCLUDE_DIR} ${wayland_INCLUDE_DIR} /usr/include/pixman-1/)
add_compile_definitions(WLR_USE_UNSTABLE)

add_custom_command(TARGET noko POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Add clangd to the src directory")

