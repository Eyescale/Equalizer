
if(GPUSD_ROOT)
  list(APPEND CMAKE_MODULE_PATH "${GPUSD_ROOT}/share/gpu-sd/CMake")
endif()
if(NOT EQUALIZER_RELEASE)
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/gpu-sd/CMake")
endif()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/gpu-sd/CMake")
list(APPEND CMAKE_MODULE_PATH /usr/share/gpu-sd/CMake)
list(APPEND CMAKE_MODULE_PATH /usr/local/share/gpu-sd/CMake)

find_package(GPUSD 1.0.2)

if(EQUALIZER_RELEASE OR (GPUSD_FOUND AND NOT GPUSD_LOCAL))
  return()
endif()

add_subdirectory(gpu-sd)
find_package(GPUSD) # re-find after add_subdirectory to find correct modules
