
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}${VERSION_MAJOR})

configure_file("version.in.h" "${CMAKE_CURRENT_BINARY_DIR}/version.h" @ONLY)

set(HEADERS
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  api.h
  module.h
  gpuInfo.h
  types.h
)

set(SOURCES
  module.cpp
  version.cpp
)

set(LIBRARY gpusd)
set(INCLUDE ${LIBRARY}${VERSION_MAJOR})

add_library(${LIBRARY} SHARED ${HEADERS} ${SOURCES})
set_target_properties(${LIBRARY} PROPERTIES VERSION ${VERSION}
                                            SOVERSION ${VERSION_MAJOR})

install(FILES ${HEADERS} DESTINATION ${INCLUDE} COMPONENT dev)
install(TARGETS ${LIBRARY}
  ARCHIVE DESTINATION lib RUNTIME DESTINATION bin LIBRARY DESTINATION lib
  COMPONENT lib)
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${INCLUDE} ${LIBRARY} WORKING_DIRECTORY \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/include)" )

add_subdirectory(agl)
add_subdirectory(glx)
add_subdirectory(dns_sd)
