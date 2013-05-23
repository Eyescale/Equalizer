# Copyright (c) 2010      Daniel Pfeifer  <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>
#                    2011 Maxim  Makhinya <maxmah@gmail.com>

set(TCLAP_TGZ ${CMAKE_CURRENT_SOURCE_DIR}/tclap.tar.gz)
set(TCLAP_DIR ${OUTPUT_INCLUDE_DIR}/tclap)

if(NOT EXISTS ${TCLAP_DIR} OR ${TCLAP_TGZ} IS_NEWER_THAN ${TCLAP_DIR})
  file(MAKE_DIRECTORY ${TCLAP_DIR})
  message(STATUS "  Extracting TCLAP to ${TCLAP_DIR}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${TCLAP_TGZ} WORKING_DIRECTORY ${TCLAP_DIR})
endif()

install(DIRECTORY ${TCLAP_DIR} DESTINATION include COMPONENT tclap)

