
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}${VERSION_MAJOR})

add_subdirectory(local)
add_subdirectory(remote)
