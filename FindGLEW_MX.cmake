# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2011 Stefan Eilemann <eile@eyescale.ch>

find_path(_glew_mx_INCLUDE_DIR GL/glew.h
  /usr/include /usr/local/include /opt/local/include)

find_library(_glew_mx_LIBRARY
  NAMES GLEWmx GLEW glew glew32
  PATHS /usr/lib /usr/local/lib)

if(_glew_mx_INCLUDE_DIR AND _glew_mx_LIBRARY)
  set(TEST_SRC ${CMAKE_BINARY_DIR}/glew_test.cpp)
  file(WRITE ${TEST_SRC}
    "#include <GL/glew.h>\n\n"
    "int main(int argc, char* argv[])\n"
    "{\n glewContextInit(0);\n}\n"
  )

  try_compile(_glew_mx_SUPPORTED ${CMAKE_BINARY_DIR}/glew_test ${TEST_SRC}
    CMAKE_FLAGS
      "-DINCLUDE_DIRECTORIES:STRING=${GLEW_INCLUDE_DIRS}"
      "-DLINK_LIBRARIES:STRING=${GLEW_LIBRARY}"
    COMPILE_DEFINITIONS -DGLEW_MX=1
    )

  if(NOT _glew_mx_SUPPORTED)
    message(STATUS "  ${GLEW_LIBRARY} does not support GLEW_MX.")
    set(_glew_mx_INCLUDE_DIR)
    set(_glew_mx_LIBRARY)
  endif(NOT _glew_mx_SUPPORTED)
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLEW_MX DEFAULT_MSG
                                  _glew_mx_LIBRARY _glew_mx_INCLUDE_DIR)

set(GLEW_MX_INCLUDE_DIRS ${_glew_mx_INCLUDE_DIR})
set(GLEW_MX_LIBRARIES ${_glew_mx_LIBRARY})

if(GLEW_MX_FOUND)
  message("-- Found GLEW_MX in ${GLEW_MX_INCLUDE_DIRS};${GLEW_MX_LIBRARIES}")
endif()

