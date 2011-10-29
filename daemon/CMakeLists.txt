
set(HEADERS)
set(SOURCES
  gpu-sd.cpp)

find_package(DNSSD REQUIRED)
include_directories(${DNSSD_INCLUDE_DIRS})

set(GPUSD_LOCAL_LIBRARIES gpusd_local)
if(APPLE)
  list(APPEND GPUSD_LOCAL_LIBRARIES gpusd_local_agl)
endif()

source_group(\\ FILES ${SOURCES} ${HEADERS})
add_executable(gpu-sd ${SOURCES} ${HEADERS})
target_link_libraries(gpu-sd ${DNSSD_LIBRARIES} ${GPUSD_LOCAL_LIBRARIES})
install(TARGETS gpu-sd DESTINATION bin COMPONENT daemon)
