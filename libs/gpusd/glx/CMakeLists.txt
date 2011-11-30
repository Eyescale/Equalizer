if(NOT X11_FOUND)
  return()
endif()

set(HEADERS
  module.h
)

set(SOURCES
  module.cpp
)

find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
include_directories(SYSTEM ${X11_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} /usr/X11R6/include)

set(LIBRARY gpusd_glx)
set(INCLUDE include/gpusd/glx)

source_group(\\ FILES CMakeLists.txt ${HEADERS} ${SOURCES})
add_library(${LIBRARY} SHARED ${HEADERS} ${SOURCES})
target_link_libraries(${LIBRARY} ${OPENGL_LIBRARIES} ${X11_LIBRARIES} gpusd)
set_target_properties(${LIBRARY} PROPERTIES VERSION ${VERSION}
  SOVERSION ${VERSION_MAJOR} FRAMEWORK TRUE PUBLIC_HEADER "${HEADERS}"
  COMPILE_DEFINITIONS "GPUSD_SHARED")
install(TARGETS ${LIBRARY}
  PUBLIC_HEADER DESTINATION ${INCLUDE} COMPONENT gpusd_dev
  ARCHIVE DESTINATION lib COMPONENT gpusd_dev
  RUNTIME DESTINATION bin COMPONENT gpusd_runtime
  LIBRARY DESTINATION lib COMPONENT gpusd_runtime
  FRAMEWORK DESTINATION lib COMPONENT gpusd_runtime)
