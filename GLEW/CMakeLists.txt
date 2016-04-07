# Copyright (c) 2011-2016 Daniel Pfeifer <daniel@pfeifer-mail.de>
#                         Stefan Eilemann <eile@eyescale.ch>

if(GLEW_MX_FOUND)
  include_directories(BEFORE SYSTEM "${GLEW_MX_INCLUDE_DIRS}")
  set(GLEW_LIBRARY ${GLEW_MX_LIBRARIES})
else()
  set(GLEW_VERSION 1.9.0)
  set(GLEW_DIR "${PROJECT_BINARY_DIR}/glew-${GLEW_VERSION}")
  set(GLEW_MX_INCLUDE_DIRS "${PROJECT_BINARY_DIR}/include/eq")
  set(GLEW_LIBRARY GLEW_MX_Equalizer)
  set(GLEW_TGZ "${CMAKE_CURRENT_SOURCE_DIR}/GLEW/glew-${GLEW_VERSION}.tgz")

  if(NOT EXISTS ${GLEW_DIR})
    execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xzf
      ${GLEW_TGZ} WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

    # 'Fix' glxew.h
    set(_glxew_h "${GLEW_DIR}/include/GL/glxew.h")
    file(READ ${_glxew_h} _glxew_h_contents)

    string(REGEX REPLACE "GL/glew.h" "eq/GL/glew.h" _glxew_h_new
      "${_glxew_h_contents}")
    string(REGEX REPLACE "GLXEW_FUN_EXPORT\n"
      "GLXEW_FUN_EXPORT GLEW_FUN_EXPORT\n" _glxew_h_new "${_glxew_h_new}")
    file(WRITE ${_glxew_h} "${_glxew_h_new}")
  endif()

  set(GLEW_INCLUDES GL/glew.h GL/glxew.h GL/wglew.h)
  set(GLEW_MX_EQUALIZER_PUBLIC_HEADERS)
  set(GLEW_MX_EQUALIZER_SOURCES "${GLEW_DIR}/src/glew.c")
  if(APPLE)
    if(EQ_AGL_USED)
      set(GLEW_MX_EQUALIZER_LINK_LIBRARIES ${OPENGL_LIBRARIES})
    else()
      set(GLEW_MX_EQUALIZER_LINK_LIBRARIES GL)
    endif()
  else()
    set(GLEW_MX_EQUALIZER_LINK_LIBRARIES ${OPENGL_gl_LIBRARY})
  endif()
  foreach(file ${GLEW_INCLUDES})
    configure_file("${GLEW_DIR}/include/${file}"
      "${PROJECT_BINARY_DIR}/include/eq/${file}" COPYONLY)
    list(APPEND GLEW_MX_EQUALIZER_PUBLIC_HEADERS
      "${PROJECT_BINARY_DIR}/include/eq/${file}")
  endforeach()

  set(GLEW_MX_EQUALIZER_INCLUDE_NAME eq/GL)
  set(GLEW_MX_EQUALIZER_OMIT_LIBRARY_HEADER ON)
  include_directories(BEFORE SYSTEM ${GLEW_MX_INCLUDE_DIRS})

  add_definitions(-DGLEW_MX=1 -DGLEW_NO_GLU -DGLEW_BUILD=1)
  common_library(${GLEW_LIBRARY})

  message(STATUS "Configured ${GLEW_LIBRARY} [${GLEW_VERSION}]")
endif()
