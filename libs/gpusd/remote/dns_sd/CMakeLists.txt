
include_directories(${DNSSD_INCLUDE_DIRS})

set(SOURCES
  module.h
  module.cpp
)

set(LIBRARY gpusd_remote_dns_sd)

add_library(${LIBRARY} SHARED ${SOURCES})
target_link_libraries(${LIBRARY} ${DNSSD_LIBRARIES} gpusd_remote)
set_target_properties(${LIBRARY} PROPERTIES VERSION ${VERSION}
  SOVERSION ${VERSION_MAJOR})

install(TARGETS ${LIBRARY}
  ARCHIVE DESTINATION lib RUNTIME DESTINATION bin LIBRARY DESTINATION lib
  COMPONENT lib)
