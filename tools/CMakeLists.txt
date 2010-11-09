# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

include(ParseArguments)
include(PurpleExpandLibraries)

macro(EQ_ADD_TOOL NAME)
  parse_arguments(THIS "SOURCES;HEADERS;LINK_LIBRARIES" "" ${ARGN})
  set(THIS_TARGET exe_${NAME})
  add_executable(${THIS_TARGET} ${THIS_SOURCES} ${THIS_HEADERS})
  purple_expand_libraries(LINK_LIBRARIES ${THIS_LINK_LIBRARIES})
  target_link_libraries(${THIS_TARGET} ${LINK_LIBRARIES})
  set_target_properties(${THIS_TARGET} PROPERTIES OUTPUT_NAME ${NAME})
  install(TARGETS ${THIS_TARGET} DESTINATION bin COMPONENT tools)
endmacro(EQ_ADD_TOOL NAME)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/examples/include)

if(WIN32)
  eq_add_tool(affinityCheck
    SOURCES
      affinityCheck/affinityCheck.cpp
    LINK_LIBRARIES
      ${OPENGL_LIBRARIES}
      ${GLEW_LIBRARY}
    )
endif(WIN32)

eq_add_tool(configTool
  HEADERS
    configTool/configTool.h
    configTool/frame.h
  SOURCES
    configTool/configTool.cpp
    configTool/writeFromFile.cpp
  LINK_LIBRARIES
    Equalizer
    EqualizerServer
  )

if(NOT WIN32) # TODO: make it compatible!
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
endif(NOT WIN32)

eq_add_tool(netperf
  SOURCES
    netperf/netperf.cpp
  LINK_LIBRARIES
    Equalizer
  )

eq_add_tool(netproxy
  SOURCES
    netproxy/netproxy.cpp
  LINK_LIBRARIES
    Equalizer
  )

eq_add_tool(windowAdmin
  SOURCES
    windowAdmin/main.cpp
  LINK_LIBRARIES
    Equalizer
    EqualizerAdmin
  )

# TODO: integrate these into EQ_ADD_TOOL
if(MSVC)
    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/affinityCheck.pdb DESTINATION bin COMPONENT dev)
    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/netperf.pdb DESTINATION bin COMPONENT dev)
endif(MSVC)
