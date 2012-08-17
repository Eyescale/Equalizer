
# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>
# gathers LSB system information

if(LSB_RELEASE)
  return()
endif()

find_program(LSB_RELEASE_EXECUTABLE lsb_release)

if(LSB_RELEASE_EXECUTABLE)
  execute_process(COMMAND ${LSB_RELEASE_EXECUTABLE} -sc
    OUTPUT_VARIABLE LSB_CODENAME OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND ${LSB_RELEASE_EXECUTABLE} -sr
    OUTPUT_VARIABLE LSB_RELEASE OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND ${LSB_RELEASE_EXECUTABLE} -si
    OUTPUT_VARIABLE LSB_DISTRIBUTOR_ID OUTPUT_STRIP_TRAILING_WHITESPACE)

  message(STATUS "LSB-Release system information: Distributor-ID: "
    "${LSB_DISTRIBUTOR_ID}  Release: ${LSB_RELEASE}  Codename: ${LSB_CODENAME}")
else()
  set(LSB_DISTRIBUTOR_ID "unknown")
  set(LSB_RELEASE "unknown")
  set(LSB_CODENAME "unknown")
endif()
