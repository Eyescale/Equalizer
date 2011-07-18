# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>

set(VMMLIB_TGZ ${CMAKE_CURRENT_SOURCE_DIR}/vmmlib.tar.gz)
set(VMMLIB_DIR ${OUTPUT_INCLUDE_DIR}/vmmlib)

if(NOT EXISTS ${VMMLIB_DIR} OR ${VMMLIB_TGZ} IS_NEWER_THAN ${VMMLIB_DIR})
  file(MAKE_DIRECTORY ${VMMLIB_DIR})
  message(STATUS "  Extracting VMML to ${VMMLIB_DIR}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${VMMLIB_TGZ} WORKING_DIRECTORY ${VMMLIB_DIR})
endif()

install(DIRECTORY ${VMMLIB_DIR} DESTINATION include COMPONENT vmmlib)

