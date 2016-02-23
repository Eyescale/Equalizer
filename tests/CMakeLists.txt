# Copyright (c) 2010-2015, Stefan Eilemann <eile@eyescale.ch>
#
# Change this number when adding tests to force a CMake run: 7

file(GLOB COMPOSITOR_IMAGES compositor/*.rgb)
file(COPY perf/images ${PROJECT_SOURCE_DIR}/examples/configs
  ${COMPOSITOR_IMAGES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB TEST_CONFIGS server/reliability/*.eqc)
make_directory(${CMAKE_CURRENT_BINARY_DIR}/reliability)
file(COPY ${TEST_CONFIGS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/reliability)

if(GLEW_MX_FOUND)
  include_directories(BEFORE SYSTEM ${GLEW_MX_INCLUDE_DIRS})
endif()

set(TEST_LIBRARIES Equalizer EqualizerAdmin EqualizerServer EqualizerFabric
  Sequel ${Boost_LIBRARIES})
include(CommonCTest)

if(APPLE) # test that only one OpenGL (X11 lib or OpenGL framework) is linked
  find_program(OTOOL otool)
  if(EQ_AGL_USED)
    set(EXCLUDED_OPENGL libGL...dylib)
    set(INCLUDED_OPENGL AGL)
  elseif(EQ_GLX_USED)
    set(EXCLUDED_OPENGL OpenGL.framework)
    set(INCLUDED_OPENGL X11)
  elseif(EQ_QT_USED)
    set(EXCLUDED_OPENGL libGL...dylib)
    set(INCLUDED_OPENGL Qt)
  else()
    message(WARNING "Equalizer build without AGL, GLX and Qt?")
    return()
  endif()

  file(WRITE ${PROJECT_BINARY_DIR}/AppleCheckOpenGL.cmake
    "execute_process(COMMAND ${OTOOL} -L bin/eqPly.app/Contents/MacOS/eqPly\n"
    "  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}\n"
    "  OUTPUT_VARIABLE EQLIBS ERROR_VARIABLE ERROR)\n"
    "if(ERROR)\n"
    "  message(FATAL_ERROR \"Can't determine link libraries of eqPly: \${ERROR}\")\n"
    "endif()\n"
    "if(EQLIBS MATCHES ${EXCLUDED_OPENGL})\n"
    "  message(FATAL_ERROR \"${EXCLUDED_OPENGL} linked to eqPly ${INCLUDED_OPENGL} build\")\n"
    "endif()\n"
    )

  add_test(NAME AppleCheckOpenGL
    COMMAND ${CMAKE_COMMAND} -P ${PROJECT_BINARY_DIR}/AppleCheckOpenGL.cmake)
  add_custom_target(AppleCheckOpenGL
    COMMAND ${CMAKE_COMMAND} -P ${PROJECT_BINARY_DIR}/AppleCheckOpenGL.cmake
    DEPENDS eqPly
    COMMENT "Verifying OpenGL link libraries of eqPly")
  add_dependencies(${PROJECT_NAME}-tests AppleCheckOpenGL)
endif()
