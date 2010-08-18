# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

set(VMMLIB_TGZ ${CMAKE_SOURCE_DIR}/externals/vmmlib.tar.gz)
set(VMMLIB_DIR ${EQ_INCLUDE_DIR}/vmmlib)

if(NOT EXISTS ${VMMLIB_DIR})
  file(MAKE_DIRECTORY ${VMMLIB_DIR})
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
    ${VMMLIB_TGZ} WORKING_DIRECTORY ${VMMLIB_DIR})
endif(NOT EXISTS ${VMMLIB_DIR})

install(DIRECTORY ${VMMLIB_DIR} DESTINATION include COMPONENT vmmlib)

