if(NOT APPLE)
  return()
endif()

set(HEADERS
  module.h
)
  
set(SOURCES
  module.cpp
)

find_library(CARBON_LIBRARY Carbon)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

set(LIBRARY gpusd_cgl)
set(INCLUDE include/gpusd/cgl)

add_library(${LIBRARY} SHARED ${HEADERS} ${SOURCES})
target_link_libraries(${LIBRARY} ${CARBON_LIBRARY} ${OPENGL_LIBRARIES} gpusd)
set_target_properties(${LIBRARY} PROPERTIES VERSION ${VERSION}
  SOVERSION ${VERSION_MAJOR})

install(FILES ${HEADERS} DESTINATION ${INCLUDE} COMPONENT dev)
install(TARGETS ${LIBRARY}
  ARCHIVE DESTINATION lib RUNTIME DESTINATION bin LIBRARY DESTINATION lib
  COMPONENT runtime)
