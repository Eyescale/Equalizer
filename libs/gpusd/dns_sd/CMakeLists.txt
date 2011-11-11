
include_directories(${DNSSD_INCLUDE_DIRS})

set(HEADERS
  module.h
)

set(SOURCES
  module.cpp
)

set(LIBRARY gpusd_dns_sd)

set(INCLUDE gpusd/dns_sd)
add_library(${LIBRARY} SHARED ${HEADERS} ${SOURCES})
target_link_libraries(${LIBRARY} ${DNSSD_LIBRARIES} gpusd)
set_target_properties(${LIBRARY} PROPERTIES VERSION ${VERSION}
  SOVERSION ${VERSION_MAJOR})

install(FILES ${HEADERS} DESTINATION ${INCLUDE} COMPONENT dev)
install(TARGETS ${LIBRARY}
  ARCHIVE DESTINATION lib RUNTIME DESTINATION bin LIBRARY DESTINATION lib
  COMPONENT lib)
