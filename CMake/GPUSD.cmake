
if(GPUSD_ROOT)
  list(APPEND CMAKE_MODULE_PATH "${GPUSD_ROOT}/share/gpu-sd/CMake")
endif()
if(NOT "$ENV{GPUSD_ROOT}" STREQUAL "")
  list(APPEND CMAKE_MODULE_PATH "$ENV{GPUSD_ROOT}/share/gpu-sd/CMake")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/gpu-sd/CMake")
list(APPEND CMAKE_MODULE_PATH /usr/share/gpu-sd/CMake)
list(APPEND CMAKE_MODULE_PATH /usr/local/share/gpu-sd/CMake)

find_package(GPUSD 1.0.3 QUIET)
