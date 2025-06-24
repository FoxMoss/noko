add_executable(noko
  main.cc
)

find_package(X11 REQUIRED)
target_link_libraries(noko ${X11_LIBRARIES})

add_custom_command(TARGET noko POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Add clangd to the src directory")

