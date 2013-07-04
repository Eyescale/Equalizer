# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2013 Stefan.Eilemann@epfl.ch

include(CMakeParseArguments)
include(PurpleInstallPDB)

macro(EQ_ADD_TOOL NAME)
  cmake_parse_arguments(THIS "" "" "SOURCES;HEADERS;LINK_LIBRARIES" ${ARGN})
  set(THIS_TARGET ${NAME})
  source_group(\\ FILES ${THIS_SOURCES} ${THIS_HEADERS})
  add_executable(${THIS_TARGET} ${THIS_SOURCES} ${THIS_HEADERS})
  target_link_libraries(${THIS_TARGET} ${THIS_LINK_LIBRARIES})
  set_target_properties(${THIS_TARGET} PROPERTIES OUTPUT_NAME ${NAME} FOLDER "Tools")
  install(TARGETS ${THIS_TARGET} DESTINATION bin COMPONENT apps)
  purple_install_pdb(${THIS_TARGET} DESTINATION bin COMPONENT apps)
endmacro(EQ_ADD_TOOL NAME)

include_directories(${CMAKE_SOURCE_DIR}/examples/include
  ${CMAKE_SOURCE_DIR}/examples/eqPly)
add_definitions(-DEQ_SYSTEM_INCLUDES) # get GL headers

if(GLEW_MX_FOUND)
  include_directories(BEFORE SYSTEM ${GLEW_MX_INCLUDE_DIRS})
  set(GLEW_LIBRARY ${GLEW_MX_LIBRARIES})
else()
  set(GLEW_LIBRARY "GLEW_MX_Equalizer")
  add_definitions(-DGLEW_STATIC=1 -DGLEW_MX=1)
endif()

if(WIN32)
  eq_add_tool(affinityCheck SOURCES affinityCheck/affinityCheck.cpp
    LINK_LIBRARIES ${GLEW_LIBRARY} ${OPENGL_gl_LIBRARY}
    )
endif(WIN32)

eq_add_tool(eqConfigTool
  HEADERS configTool/configTool.h configTool/frame.h
  SOURCES configTool/configTool.cpp configTool/writeFromFile.cpp
  LINK_LIBRARIES Equalizer EqualizerServer
  )

eq_add_tool(eqThreadAffinity SOURCES threadAffinity/threadAffinity.cpp
  LINK_LIBRARIES Equalizer EqualizerServer
  )

eq_add_tool(eqPlyConverter
  HEADERS
    ../examples/eqPly/ply.h
    ../examples/eqPly/vertexBufferBase.h
    ../examples/eqPly/vertexBufferData.h
    ../examples/eqPly/vertexBufferLeaf.h
    ../examples/eqPly/vertexBufferNode.h
    ../examples/eqPly/vertexBufferRoot.h
    ../examples/eqPly/vertexBufferState.h
    ../examples/eqPly/vertexData.h
  SOURCES eqPlyConverter/main.cpp
    ../examples/eqPly/plyfile.cpp
    ../examples/eqPly/vertexBufferBase.cpp
    ../examples/eqPly/vertexBufferLeaf.cpp
    ../examples/eqPly/vertexBufferNode.cpp
    ../examples/eqPly/vertexBufferRoot.cpp
    ../examples/eqPly/vertexBufferState.cpp
    ../examples/eqPly/vertexData.cpp
  LINK_LIBRARIES Equalizer
  )

eq_add_tool(eVolveConverter
  HEADERS
    eVolveConverter/codebase.h
    eVolveConverter/ddsbase.h
    eVolveConverter/eVolveConverter.h
    eVolveConverter/hlp.h
  SOURCES
    eVolveConverter/eVolveConverter.cpp
    eVolveConverter/ddsbase.cpp
  )

eq_add_tool(eqWindowAdmin
  SOURCES windowAdmin/main.cpp
  LINK_LIBRARIES EqualizerAdmin
  )

eq_add_tool(eqServer
  SOURCES server/eqServer.cpp
  LINK_LIBRARIES EqualizerServer
  )
