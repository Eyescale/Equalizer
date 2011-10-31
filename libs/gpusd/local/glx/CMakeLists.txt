if(NOT X11_FOUND)
  return()
endif()

set(SOURCES
  module.h
  module.cpp
)

set(LIBRARY gpusd_local_glx)

include_directories(SYSTEM ${X11_INCLUDE_DIR})
add_library(${LIBRARY} SHARED ${SOURCES})
target_link_libraries(${LIBRARY} ${CARBON_LIBRARY} ${OPENGL_LIBRARIES} ${X11_LIBRARIES} gpusd_local)
set_target_properties(${LIBRARY} PROPERTIES VERSION ${VERSION}
  SOVERSION ${VERSION_MAJOR})

install(TARGETS ${LIBRARY}
  ARCHIVE DESTINATION lib RUNTIME DESTINATION bin LIBRARY DESTINATION lib
  COMPONENT lib)
