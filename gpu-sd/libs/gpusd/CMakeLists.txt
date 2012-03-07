
configure_file("version.in.h" "${CMAKE_CURRENT_BINARY_DIR}/version.h" @ONLY)

set(HEADERS
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  api.h
  filter.h
  module.h
  gpuInfo.h
  types.h
)

set(SOURCES
  filter.cpp
  module.cpp
  version.cpp
)

set(LIBRARY gpusd)
set(INCLUDE include/${LIBRARY})

source_group(\\ FILES CMakeLists.txt ${HEADERS} ${SOURCES})
add_library(${LIBRARY} SHARED ${HEADERS} ${SOURCES})
set_target_properties(${LIBRARY} PROPERTIES
  VERSION ${VERSION} SOVERSION ${VERSION_MAJOR}
  PUBLIC_HEADER "${HEADERS}" COMPILE_DEFINITIONS "GPUSD_SHARED")
if(CMAKE_PROJECT_NAME MATCHES "gpusd")
    install(TARGETS ${LIBRARY}
      PUBLIC_HEADER DESTINATION ${INCLUDE} COMPONENT dev
      ARCHIVE DESTINATION lib COMPONENT dev
      RUNTIME DESTINATION bin COMPONENT runtime
      LIBRARY DESTINATION lib COMPONENT runtime)
endif()

add_subdirectory(cgl)
add_subdirectory(glx)
add_subdirectory(wgl)
add_subdirectory(dns_sd)
