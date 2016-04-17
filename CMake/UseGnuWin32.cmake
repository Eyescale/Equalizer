# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

set(GNUWIN32_NAME GnuWin32)
set(GNUWIN32_TGZ ${PROJECT_SOURCE_DIR}/CMake/${GNUWIN32_NAME}.tar.gz)
set(GNUWIN32_DIR ${PROJECT_BINARY_DIR}/${GNUWIN32_NAME})

if(NOT EXISTS ${GNUWIN32_DIR})
  message(STATUS "  Extracting GnuWin32 to ${GNUWIN32_DIR}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
    ${GNUWIN32_TGZ} WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
endif(NOT EXISTS ${GNUWIN32_DIR})

file(WRITE ${PROJECT_BINARY_DIR}/include/unistd.h "\n")

set(BISON_EXECUTABLE ${GNUWIN32_DIR}/bin/bison.bat)
set(FLEX_EXECUTABLE ${GNUWIN32_DIR}/bin/flex.exe)
