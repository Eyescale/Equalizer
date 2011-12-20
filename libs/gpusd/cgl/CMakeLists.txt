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

source_group(\\ FILES CMakeLists.txt ${HEADERS} ${SOURCES})
add_library(${LIBRARY} SHARED ${HEADERS} ${SOURCES})
target_link_libraries(${LIBRARY} ${CARBON_LIBRARY} ${OPENGL_LIBRARIES} gpusd)
set_target_properties(${LIBRARY} PROPERTIES VERSION ${VERSION}
  SOVERSION ${VERSION_MAJOR} PUBLIC_HEADER "${HEADERS}"
  COMPILE_DEFINITIONS "GPUSD_SHARED")
if(CMAKE_PROJECT_NAME MATCHES "gpusd")
  install(TARGETS ${LIBRARY}
    PUBLIC_HEADER DESTINATION ${INCLUDE} COMPONENT dev
    ARCHIVE DESTINATION lib COMPONENT dev
    RUNTIME DESTINATION bin COMPONENT runtime
    LIBRARY DESTINATION lib COMPONENT runtime)
endif()
