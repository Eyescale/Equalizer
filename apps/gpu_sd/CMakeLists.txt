
if(NOT DNSSD_FOUND)
  return()
endif()

set(HEADERS)
set(SOURCES
  gpu_sd.cpp)

include_directories(${DNSSD_INCLUDE_DIRS})

set(GPUSD_LIBRARIES gpusd)
if(APPLE)
  add_definitions(-DGPUSD_CGL)
  list(APPEND GPUSD_LIBRARIES gpusd_cgl)
endif()
if(X11_FOUND)
  add_definitions(-DGPUSD_GLX)
  list(APPEND GPUSD_LIBRARIES gpusd_glx)
endif()
if(WIN32)
  add_definitions(-DGPUSD_WGL)
  list(APPEND GPUSD_LIBRARIES gpusd_wgl ws2_32)
endif()

source_group(\\ FILES ${SOURCES} ${HEADERS})
add_executable(gpu_sd ${SOURCES} ${HEADERS})
target_link_libraries(gpu_sd ${DNSSD_LIBRARIES} ${GPUSD_LIBRARIES})
install(TARGETS gpu_sd DESTINATION bin COMPONENT daemon)
