
if(EXISTS "${CMAKE_SOURCE_DIR}/gpu-sd/CMakeLists.txt")
  set(FOUND_GPUSD 1)
  return()
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/gpu-sd")
  message(WARNING "${CMAKE_SOURCE_DIR}/gpu-sd is not initialized correctly, no automatic configuration")
  return()
endif()

if(NOT ${EQ_REVISION})
  message(WARNING "git not found for 'git submodule init', no automatic configuration")
  return()
endif()

EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} submodule init
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/gpu-sd/CMakeLists.txt")
  message(WARNING "git submodule init failed, no automatic configuration")
  return()
endif()

set(FOUND_GPUSD 1)
