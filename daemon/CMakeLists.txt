
set(HEADERS)
set(SOURCES
  gpu-sd.cpp)

find_package(DNSSD REQUIRED)
include_directories(${DNSSD_INCLUDE_DIRS})

source_group(\\ FILES ${SOURCES} ${HEADERS})
add_executable(gpu-sd ${SOURCES} ${HEADERS})
target_link_libraries(gpu-sd ${DNSSD_LIBRARIES})
install(TARGETS gpu-sd DESTINATION bin COMPONENT daemon)
