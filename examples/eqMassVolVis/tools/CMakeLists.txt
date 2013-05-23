
# Copyright (c)  2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#                2011 Maxim Makhinya <maxmah@gmail.com>
#                2012 David Steiner  <steiner@ifi.uzh.ch>

include(ParseArguments)
include(PurpleInstallPDB)
include(PurpleExpandLibraries)

set(MSV_LIB "${PROJECT_BINARY_DIR}/lib/libmsv.so")

macro(MASSVOL_ADD_TOOL NAME)
  parse_arguments(THIS "SOURCES;HEADERS;LINK_LIBRARIES" "" ${ARGN})
  set(THIS_TARGET ${NAME})
  source_group(\\ FILES ${THIS_SOURCES} ${THIS_HEADERS})
  add_executable(${THIS_TARGET} ${THIS_SOURCES} ${THIS_HEADERS})
  purple_expand_libraries(LINK_LIBRARIES ${THIS_LINK_LIBRARIES})
  target_link_libraries(${THIS_TARGET} ${LINK_LIBRARIES})
  set_target_properties(${THIS_TARGET} PROPERTIES OUTPUT_NAME ${NAME} FOLDER "Tools")
  install(TARGETS ${THIS_TARGET} DESTINATION bin COMPONENT tools)
  purple_install_pdb(${THIS_TARGET} DESTINATION bin COMPONENT tools)
endmacro(MASSVOL_ADD_TOOL NAME)

#include_directories(${CMAKE_SOURCE_DIR}/examples/include)

if(GLEW_MX_FOUND)
  include_directories(BEFORE SYSTEM ${GLEW_MX_INCLUDE_DIR})
  set(GLEW_LIBRARY ${GLEW_MX_LIBRARIES})
else()
  set(GLEW_LIBRARY "GLEW_MX-Equalizer")
endif()

massvol_add_tool(dataConverter
  HEADERS
    dataConverter/dataConverter.h
    dataConverter/dataManipulation.h
  SOURCES
    dataConverter/dataConverter.cpp
    dataConverter/dataManipulation.cpp
    dataConverter/main.cpp
  LINK_LIBRARIES
    ${MSV_LIB}
  )
add_dependencies(dataConverter ${MSV_LIB})

# if(CUDA_FOUND)
#  include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})
#  list(APPEND EQUALIZER_DEFINES EQ_USE_CUDA)
#  message(STATUS "Using CUDA from: ${CUDA_INCLUDE_DIRS}")
# endif()

add_subdirectory(visGUI)
