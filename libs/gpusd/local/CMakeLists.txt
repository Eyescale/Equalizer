
find_package(OpenGL REQUIRED)
configure_file("version.in.h" "${CMAKE_CURRENT_BINARY_DIR}/version.h" @ONLY)

set(HEADERS
  "${CMAKE_CURRENT_BINARY_DIR}/version.h"
  api.h
  gpuInfo.h
  module.h
)

set(SOURCES
  gpuInfo.cpp
  module.cpp
  version.cpp
)

set(LIBRARY gpusd_local)
set(LIB_INCLUDE gpusd${VERSION_MAJOR}/local)

add_library(${LIBRARY} SHARED ${HEADERS} ${SOURCES})
set_target_properties(${LIBRARY} PROPERTIES VERSION ${VERSION}
  SOVERSION ${VERSION_MAJOR})

install(FILES ${HEADERS} DESTINATION include/${LIB_INCLUDE} COMPONENT dev)
install(TARGETS ${LIBRARY}
  ARCHIVE DESTINATION lib RUNTIME DESTINATION bin LIBRARY DESTINATION lib
  COMPONENT lib)
install_symlink(${LIB_INCLUDE} ${LIBRARY} include)

add_subdirectory(agl)
