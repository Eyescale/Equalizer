
configure_file("version.in.h" "${CMAKE_CURRENT_BINARY_DIR}/version.h" @ONLY)

set(HEADERS
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  api.h
  module.h
)

set(SOURCES
  module.cpp
  version.cpp
)

set(LIBRARY gpusd_remote)

add_library(${LIBRARY} SHARED ${HEADERS} ${SOURCES})
set_target_properties(${LIBRARY} PROPERTIES VERSION ${VERSION}
  SOVERSION ${VERSION_MAJOR})

install(FILES ${HEADERS} DESTINATION include/gpusd${VERSION_MAJOR}/remote
  COMPONENT dev)
install(TARGETS ${LIBRARY}
  ARCHIVE DESTINATION lib RUNTIME DESTINATION bin LIBRARY DESTINATION lib
  COMPONENT lib)

add_subdirectory(dns_sd)