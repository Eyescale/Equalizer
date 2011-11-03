
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}${VERSION_MAJOR})

set(HEADERS
  gpuInfo.h
  types.h
)

set(INCLUDE gpusd${VERSION_MAJOR})
install(FILES ${HEADERS} DESTINATION ${INCLUDE} COMPONENT dev)
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${INCLUDE} gpusd WORKING_DIRECTORY \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/include)" )

add_subdirectory(local)
add_subdirectory(remote)
