if(NOT WIN32)
  return()
endif()

set(HEADERS
  module.h
)

set(SOURCES
  module.cpp
)

find_package(OpenGL REQUIRED)

set(LIBRARY gpusd_wgl)
set(INCLUDE include/gpusd/wgl)

source_group(\\ FILES CMakeLists.txt ${HEADERS} ${SOURCES})
add_library(${LIBRARY} SHARED ${HEADERS} ${SOURCES})
target_link_libraries(${LIBRARY} ${OPENGL_LIBRARIES} gpusd)
set_target_properties(${LIBRARY} PROPERTIES VERSION ${VERSION}
  SOVERSION ${VERSION_MAJOR} COMPILE_DEFINITIONS "GPUSD_SHARED")

install(FILES ${HEADERS} DESTINATION ${INCLUDE} COMPONENT dev)
install(TARGETS ${LIBRARY}
  ARCHIVE DESTINATION lib RUNTIME DESTINATION bin LIBRARY DESTINATION lib
  COMPONENT lib)
