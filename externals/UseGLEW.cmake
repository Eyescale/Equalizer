# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2011 Stefan Eilemann <eile@eyescale.ch>

find_path(GLEW_INCLUDE_DIRS GL/glew.h
  /usr/include
  /usr/local/include
  /opt/local/include
  )

find_library(GLEW_LIBRARY
  NAMES
    GLEWmx
    GLEW
    glew
    glew32
  PATHS
    /usr/lib
    /usr/local/lib
  )

set(GLEW_FOUND FALSE)
if(GLEW_INCLUDE_DIRS AND GLEW_LIBRARY)
  message(STATUS "Found GLEW: ${GLEW_LIBRARY}")
  set(GLEW_FOUND TRUE)
endif(GLEW_INCLUDE_DIRS AND GLEW_LIBRARY)

if(GLEW_FOUND)
  set(TEST_SRC ${CMAKE_BINARY_DIR}/glew_test.cpp)
  file(WRITE ${TEST_SRC}
    "#include <GL/glew.h>\n\n"
    "int main(int argc, char* argv[])\n"
    "{\n glewContextInit(0);\n}\n"
  )

  try_compile(GLEW_MX_SUPPORTED ${CMAKE_BINARY_DIR}/glew_test ${TEST_SRC}
    CMAKE_FLAGS
      "-DINCLUDE_DIRECTORIES:STRING=${GLEW_INCLUDE_DIRS}"
      "-DLINK_LIBRARIES:STRING=${GLEW_LIBRARY}"
    COMPILE_DEFINITIONS -DGLEW_MX=1
    )

  if(NOT GLEW_MX_SUPPORTED)
    message(STATUS "  This library does not support GLEW_MX.")
  endif(NOT GLEW_MX_SUPPORTED)
endif(GLEW_FOUND)

if(NOT GLEW_FOUND OR NOT GLEW_MX_SUPPORTED)
  set(EQ_GLEW_INTERNAL TRUE)

  set(GLEW_NAME glew-1.5.8)
  set(GLEW_TGZ ${CMAKE_SOURCE_DIR}/externals/${GLEW_NAME}.tgz)
  set(GLEW_DIR ${CMAKE_BINARY_DIR}/${GLEW_NAME})

  if(NOT EXISTS ${GLEW_DIR})
    message(STATUS "  Extracting GLEW to ${GLEW_DIR}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
      ${GLEW_TGZ} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    # 'Fix' glxew.h
    set(_glxew_h "${GLEW_DIR}/include/GL/glxew.h")
    file(READ ${_glxew_h} _glxew_h_contents)

    string(REGEX REPLACE "GL/glew.h" "eq/GL/glew.h"
           _glxew_h_new "${_glxew_h_contents}")
    file(WRITE ${_glxew_h} "${_glxew_h_new}")
  endif(NOT EXISTS ${GLEW_DIR})

  set(GLEW_INCLUDE_DIRS ${GLEW_DIR}/include)
  set(GLEW_LIBRARY ${GLEW_NAME}-MX-Equalizer)

  set(GLEW_INCLUDES GL/glew.h GL/glxew.h GL/wglew.h)

  add_definitions(-DGLEW_STATIC=1 -DGLEW_MX=1)
  add_library(${GLEW_LIBRARY} STATIC ${GLEW_DIR}/src/glew.c)
  set_target_properties(${GLEW_LIBRARY} PROPERTIES FOLDER "Externals")

  if(EQ_GLX_USED)
    add_definitions(-DGLEW_APPLE_GLX=1)
  endif(EQ_GLX_USED)
  
  foreach(file ${GLEW_INCLUDES})
    configure_file(${GLEW_INCLUDE_DIRS}/${file} include/eq/${file} COPYONLY)
    install(FILES ${GLEW_INCLUDE_DIRS}/${file} DESTINATION include/eq/GL COMPONENT eqdev)
  endforeach()
  install(TARGETS ${GLEW_LIBRARY} ARCHIVE DESTINATION lib COMPONENT eqdev)
endif(NOT GLEW_FOUND OR NOT GLEW_MX_SUPPORTED)

include_directories(BEFORE SYSTEM ${GLEW_INCLUDE_DIRS})
