# sets CMAKE_MODULE_INSTALL_PATH to where CMake script should be installed

if(NOT CMAKE_MODULE_INSTALL_PATH)
  if(MSVC)
    set(CMAKE_MODULE_INSTALL_PATH ${CMAKE_PROJECT_NAME}/CMake)
  else()
    set(CMAKE_MODULE_INSTALL_PATH share/${CMAKE_PROJECT_NAME}/CMake)
  endif()
endif()
