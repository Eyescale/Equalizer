
# adds directories to the CMAKE_MODULE_PATH for cmake scripts copied
# from new cmake versions for old cmake installations.

if(CMAKE_VERSION VERSION_LESS 2.8)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/2.8)
endif()
if(CMAKE_VERSION VERSION_LESS 2.8.3)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/2.8.3)
endif()
