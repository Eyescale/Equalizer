# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2011-2013 Stefan Eilemann <eile@eyescale.ch>

if(GLEW_MX_FIND_QUIETLY)
  set(_glew_mx_output)
else()
  set(_glew_mx_output 1)
endif()

find_path(_glew_mx_INCLUDE_DIR GL/glew.h
  /usr/include /usr/local/include /opt/local/include)

find_library(_glew_mx_LIBRARY
  NAMES GLEWmx GLEW glew glew32
  PATHS /usr/lib /usr/local/lib)

if(_glew_mx_INCLUDE_DIR AND _glew_mx_LIBRARY)
  set(TEST_SRC ${CMAKE_BINARY_DIR}/glew_test.cpp)
  file(WRITE ${TEST_SRC}
    "#include <GL/glew.h>\n"
    "int main( )\n"
    "{\n"
    "  glewContextInit(0);\n"
    "}\n"
    )

  try_compile(_glew_mx_SUPPORTED ${CMAKE_BINARY_DIR}/glew_test ${TEST_SRC}
    CMAKE_FLAGS
    "-DINCLUDE_DIRECTORIES:STRING=${_glew_mx_INCLUDE_DIR}"
    "-DLINK_LIBRARIES:STRING=${_glew_mx_LIBRARY}"
    COMPILE_DEFINITIONS -DGLEW_MX=1
    )

  if(NOT _glew_mx_SUPPORTED)
    if(_glew_mx_output)
      message(STATUS "  ${_glew_mx_LIBRARY} does not support GLEW_MX.")
    endif()
    set(_glew_mx_INCLUDE_DIR 0)
    set(_glew_mx_LIBRARY 0)
  elseif(X11_FOUND)
    file(WRITE ${TEST_SRC}
      "#include <GL/glxew.h>\n"
      "int main( )\n"
      "{\n"
      "  glxewContextInit(0);\n"
      "}\n"
      )

    try_compile(_glxew_mx_SUPPORTED ${CMAKE_BINARY_DIR}/glew_test ${TEST_SRC}
      CMAKE_FLAGS
      "-DINCLUDE_DIRECTORIES:STRING=${_glew_mx_INCLUDE_DIR}"
      "-DLINK_LIBRARIES:STRING=${_glew_mx_LIBRARY}"
      COMPILE_DEFINITIONS -DGLEW_MX=1
      )
    if(NOT _glxew_mx_SUPPORTED)
      if(_glew_mx_output)
        message(STATUS "  ${_glew_mx_LIBRARY} is missing glxewContextInit().")
      endif()
      set(_glew_mx_INCLUDE_DIR 0)
      set(_glew_mx_LIBRARY 0)
    endif()
  endif()
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLEW_MX DEFAULT_MSG
                                  _glew_mx_LIBRARY _glew_mx_INCLUDE_DIR)

set(GLEW_MX_INCLUDE_DIRS ${_glew_mx_INCLUDE_DIR})
set(GLEW_MX_LIBRARIES ${_glew_mx_LIBRARY})
if(GLEW_MX_FOUND AND _glew_mx_output)
  message(STATUS
    "Found GLEW_MX in ${GLEW_MX_INCLUDE_DIRS};${GLEW_MX_LIBRARIES}")
endif()
