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
include_directories(SYSTEM ${X11_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} /usr/X11R6/include)

set(LIBRARY gpusd_glx)
set(INCLUDE gpusd/glx)

add_library(${LIBRARY} SHARED ${HEADERS} ${SOURCES})
target_link_libraries(${LIBRARY} ${CARBON_LIBRARY} ${OPENGL_LIBRARIES} ${X11_LIBRARIES} gpusd)
set_target_properties(${LIBRARY} PROPERTIES VERSION ${VERSION}
  SOVERSION ${VERSION_MAJOR})

install(FILES ${HEADERS} DESTINATION ${INCLUDE} COMPONENT dev)
install(TARGETS ${LIBRARY}
  ARCHIVE DESTINATION lib RUNTIME DESTINATION bin LIBRARY DESTINATION lib
  COMPONENT lib)
