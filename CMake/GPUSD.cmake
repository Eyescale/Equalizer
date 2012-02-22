
if(GPUSD_ROOT)
  list(APPEND CMAKE_MODULE_PATH "${GPUSD_ROOT}/share/gpu-sd/CMake")
endif()
if(NOT EQUALIZER_RELEASE)
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/gpu-sd/CMake")
endif()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/gpu-sd/CMake")
list(APPEND CMAKE_MODULE_PATH /usr/share/gpu-sd/CMake)
list(APPEND CMAKE_MODULE_PATH /usr/local/share/gpu-sd/CMake)

find_package(GPUSD 1.0.3)

if(EQUALIZER_RELEASE OR (GPUSD_FOUND AND NOT GPUSD_LOCAL))
  return()
endif()

if(GIT_FOUND)
  execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else()
  message(STATUS "git not found, automatic submodule configuration not done")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/gpu-sd/CMakeLists.txt")
  add_subdirectory(gpu-sd)
  find_package(GPUSD) # re-find after add_subdirectory to find correct modules
else()
  message(WARNING "git submodule update failed, no automatic configuration")
endif()
