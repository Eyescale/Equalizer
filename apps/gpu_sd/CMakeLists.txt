
set(HEADERS)
set(SOURCES
  gpu_sd.cpp)

include_directories(${DNSSD_INCLUDE_DIRS})

set(GPUSD_LOCAL_LIBRARIES gpusd_local)
if(APPLE)
  list(APPEND GPUSD_LOCAL_LIBRARIES gpusd_local_agl)

  # No 64 bit support for AGL
  if(_CMAKE_OSX_MACHINE MATCHES "ppc")
    set(CMAKE_OSX_ARCHITECTURES "ppc")
  else()
    set(CMAKE_OSX_ARCHITECTURES "i386")
  endif()
  message(STATUS "Building gpu_sd for ${CMAKE_OSX_ARCHITECTURES}")
endif()

if(X11_FOUND)
  list(APPEND GPUSD_LOCAL_LIBRARIES gpusd_local_glx)
endif()

source_group(\\ FILES ${SOURCES} ${HEADERS})
add_executable(gpu_sd ${SOURCES} ${HEADERS})
target_link_libraries(gpu_sd ${DNSSD_LIBRARIES} ${GPUSD_LOCAL_LIBRARIES})
install(TARGETS gpu_sd DESTINATION bin COMPONENT daemon)
