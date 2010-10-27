# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/examples/include)

macro(ADD_TOOL NAME)
  add_executable(${NAME} ${ARGN})
  list(APPEND TOOLS ${NAME})
endmacro(ADD_TOOL NAME)

set(TOOLS)

if(WIN32)
  add_tool(affinityCheck affinityCheck/affinityCheck.cpp)
  target_link_libraries(affinityCheck ${OPENGL_LIBRARIES} ${GLEW_LIBRARY})
endif(WIN32)

add_tool(configTool
  configTool/configTool.cpp
  configTool/writeFromFile.cpp
  )
target_link_libraries(configTool Equalizer EqualizerServer)

if(NOT WIN32) # TODO: make it compatible!
  add_tool(eVolveConverter
    eVolveConverter/eVolveConverter.cpp
    eVolveConverter/ddsbase.cpp
    )
endif(NOT WIN32)

add_tool(netperf netperf/netperf.cpp)
target_link_libraries(netperf Equalizer)

add_tool(netproxy netproxy/netproxy.cpp)
target_link_libraries(netproxy Equalizer)

add_tool(windowAdmin windowAdmin/main.cpp)
target_link_libraries(windowAdmin Equalizer EqualizerAdmin)

install(TARGETS ${TOOLS} DESTINATION "bin${EQ_INSTALL_POSTFIX}" COMPONENT tools)
if(MSVC)
    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/affinityCheck.pdb DESTINATION "bin${EQ_INSTALL_POSTFIX}" COMPONENT dev)
    install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/netperf.pdb DESTINATION "bin${EQ_INSTALL_POSTFIX}" COMPONENT dev)
endif(MSVC)
