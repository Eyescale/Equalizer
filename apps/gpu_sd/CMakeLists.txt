
set(HEADERS)
set(SOURCES
  gpu_sd.cpp)

include_directories(${DNSSD_INCLUDE_DIRS})

set(GPUSD_LIBRARIES gpusd)
if(APPLE)
  add_definitions(-DGPUSD_AGL)
  list(APPEND GPUSD_LIBRARIES gpusd_agl)
endif()
if(X11_FOUND)
  add_definitions(-DGPUSD_GLX)
  list(APPEND GPUSD_LIBRARIES gpusd_glx)
endif()

source_group(\\ FILES ${SOURCES} ${HEADERS})
add_executable(gpu_sd ${SOURCES} ${HEADERS})
target_link_libraries(gpu_sd ${DNSSD_LIBRARIES} ${GPUSD_LIBRARIES})
install(TARGETS gpu_sd DESTINATION bin COMPONENT daemon)
