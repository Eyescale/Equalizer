# sets ${SYSTEM}

if(WIN32)
  set(SYSTEM Win32)
endif(WIN32)
if(APPLE)
  set(SYSTEM Darwin)
endif(APPLE)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(SYSTEM Linux)
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")
if(NOT SYSTEM)
  message(FATAL_ERROR "Unable to determine OS")
endif()
