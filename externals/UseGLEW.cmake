# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

# TODO: ckeck whether the installed GLEW library has `glewContextInit'
#       if so, use it. otherwise compile from source

#find_path(GLEW_INCLUDE_DIRS GL/glew.h
#  /usr/include
#  /usr/local/include
#  /opt/local/include
#  )
#
#find_library(GLEW_LIBRARY
#  NAMES
#    glew
#    GLEW
#    glew32
#  PATHS
#    /usr/lib
#    /usr/local/lib
#  )
#
#if(NOT GLEW_INCLUDE_DIRS OR NOT GLEW_LIBRARY)
#
#  message(STATUS "GLEW was NOT found on the system. I will build it.")

  set(GLEW_NAME glew-1.5.5)
  set(GLEW_TGZ ${CMAKE_SOURCE_DIR}/externals/${GLEW_NAME}.tgz)
  set(GLEW_DIR ${CMAKE_BINARY_DIR}/${GLEW_NAME})

  if(NOT EXISTS ${GLEW_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
      ${GLEW_TGZ} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  endif(NOT EXISTS ${GLEW_DIR})

  set(GLEW_INCLUDE_DIRS ${GLEW_DIR}/include)
  set(GLEW_LIBRARY glew)

  add_definitions(-DGLEW_STATIC=1 -DGLEW_MX=1)
  add_library(${GLEW_LIBRARY} STATIC ${GLEW_DIR}/src/glew.c)

#endif(NOT GLEW_INCLUDE_DIRS OR NOT GLEW_LIBRARY)

include_directories(BEFORE SYSTEM ${GLEW_INCLUDE_DIRS})
