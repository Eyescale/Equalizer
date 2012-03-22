# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

set(GNUWIN32_NAME GnuWin32)
set(GNUWIN32_TGZ ${CMAKE_SOURCE_DIR}/CMake/${GNUWIN32_NAME}.tar.gz)
set(GNUWIN32_DIR ${CMAKE_BINARY_DIR}/${GNUWIN32_NAME})

if(NOT EXISTS ${GNUWIN32_DIR})
  message(STATUS "  Extracting GnuWin32 to ${GNUWIN32_DIR}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
    ${GNUWIN32_TGZ} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif(NOT EXISTS ${GNUWIN32_DIR})

file(WRITE ${OUTPUT_INCLUDE_DIR}/unistd.h "\n")

set(BISON_EXECUTABLE ${GNUWIN32_DIR}/bin/bison.bat)
set(FLEX_EXECUTABLE ${GNUWIN32_DIR}/bin/flex.exe)
