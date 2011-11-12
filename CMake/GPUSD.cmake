
if(EXISTS "${CMAKE_SOURCE_DIR}/gpu-sd/CMakeLists.txt")
  set(FOUND_GPUSD 1)
endif()

if(${EQ_REVISION} STREQUAL "")
  message(WARNING "git not found, automatic submodule configuration not done")
  return()
endif()

EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} submodule update --init
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/gpu-sd/CMakeLists.txt")
  message(WARNING "git submodule update failed, no automatic configuration")
  return()
endif()

set(FOUND_GPUSD 1)
