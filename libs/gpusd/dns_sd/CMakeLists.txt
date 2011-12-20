
if(NOT DNSSD_FOUND)
  return()
endif()

set(HEADERS
  module.h
)

set(SOURCES
  module.cpp
)

include_directories(${DNSSD_INCLUDE_DIRS})

set(LIBRARY gpusd_dns_sd)
set(INCLUDE include/gpusd/dns_sd)

source_group(\\ FILES CMakeLists.txt ${HEADERS} ${SOURCES})
add_library(${LIBRARY} SHARED ${HEADERS} ${SOURCES})
if(WIN32)
  target_link_libraries(${LIBRARY} ${DNSSD_LIBRARIES} gpusd ws2_32)
else()
  target_link_libraries(${LIBRARY} ${DNSSD_LIBRARIES} gpusd)
endif()
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
