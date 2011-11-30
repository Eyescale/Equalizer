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
include_directories(${OPENGL_INCLUDE_DIR})

set(LIBRARY gpusd_wgl)
set(INCLUDE include/gpusd/wgl)

source_group(\\ FILES CMakeLists.txt ${HEADERS} ${SOURCES})
add_library(${LIBRARY} SHARED ${HEADERS} ${SOURCES})
target_link_libraries(${LIBRARY} ${OPENGL_LIBRARIES} gpusd)
set_target_properties(${LIBRARY} PROPERTIES VERSION ${VERSION}
  SOVERSION ${VERSION_MAJOR} FRAMEWORK TRUE PUBLIC_HEADER "${HEADERS}"
  COMPILE_DEFINITIONS "GPUSD_SHARED")
install(TARGETS ${LIBRARY}
  PUBLIC_HEADER DESTINATION ${INCLUDE} COMPONENT gpusd_dev
  ARCHIVE DESTINATION lib COMPONENT gpusd_dev
  RUNTIME DESTINATION bin COMPONENT gpusd_runtime
  LIBRARY DESTINATION lib COMPONENT gpusd_runtime
  FRAMEWORK DESTINATION lib COMPONENT gpusd_runtime)
