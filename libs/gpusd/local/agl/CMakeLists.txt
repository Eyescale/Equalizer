if(NOT APPLE)
  return()
endif()

set(SOURCES
  module.h
  module.cpp
)

find_library(CARBON_LIBRARY Carbon)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

set(LIBRARY gpusd_local_agl)

# No 64 bit support for AGL
if(_CMAKE_OSX_MACHINE MATCHES "ppc")
  set(CMAKE_OSX_ARCHITECTURES "ppc")
else()
  set(CMAKE_OSX_ARCHITECTURES "i386")
endif()
message(STATUS "Building ${LIBRARY} for ${CMAKE_OSX_ARCHITECTURES}")

add_library(${LIBRARY} SHARED ${SOURCES})
target_link_libraries(${LIBRARY} ${CARBON_LIBRARY} ${OPENGL_LIBRARIES} gpusd_local)
set_target_properties(${LIBRARY} PROPERTIES VERSION ${VERSION}
  SOVERSION ${VERSION_MAJOR})

install(TARGETS ${LIBRARY}
  ARCHIVE DESTINATION lib RUNTIME DESTINATION bin LIBRARY DESTINATION lib
  COMPONENT lib)
