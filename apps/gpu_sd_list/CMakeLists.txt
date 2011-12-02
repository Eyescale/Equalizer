if(NOT DNSSD_FOUND)
  return()
endif()

set(HEADERS)
set(SOURCES
  gpu_sd_list.cpp
)

source_group(\\ FILES ${SOURCES} ${HEADERS})
add_executable(gpu_sd_list ${SOURCES} ${HEADERS})
target_link_libraries(gpu_sd_list gpusd_dns_sd)
install(TARGETS gpu_sd_list DESTINATION bin COMPONENT gpusdtools)
