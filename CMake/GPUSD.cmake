
find_package(GPUSD)
if(EQUALIZER_RELEASE)
  return()
endif()

if(GIT_FOUND)
  execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else()
  message(STATUS "git not found, automatic submodule configuration not done")
endif()

if(GPUSD_FOUND)
  return()
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/gpu-sd/CMakeLists.txt")
  find_package(GPUSD)
else()
  message(WARNING "git submodule update failed, no automatic configuration")
endif()
