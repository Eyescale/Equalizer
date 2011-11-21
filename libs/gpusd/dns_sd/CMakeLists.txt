
if(NOT DNSSD_FOUND)
  return()
endif()

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
if(WIN32)
  target_link_libraries(${LIBRARY} ${DNSSD_LIBRARIES} gpusd ws2_32)
else()
  target_link_libraries(${LIBRARY} ${DNSSD_LIBRARIES} gpusd)
endif()
set_target_properties(${LIBRARY} PROPERTIES VERSION ${VERSION}
  SOVERSION ${VERSION_MAJOR} COMPILE_DEFINITIONS "GPUSD_SHARED")

install(FILES ${HEADERS} DESTINATION ${INCLUDE} COMPONENT dev)
install(TARGETS ${LIBRARY}
  ARCHIVE DESTINATION lib RUNTIME DESTINATION bin LIBRARY DESTINATION lib
  COMPONENT runtime)
