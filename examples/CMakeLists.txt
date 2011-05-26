# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>

if(NOT EQUALIZER_BUILD_EXAMPLES)
  return()
endif()

include(ParseArguments)
include(PurpleExpandLibraries)
include(EqStringifyShaders)

function(EQ_ADD_EXAMPLE NAME)
  parse_arguments(THIS "SOURCES;HEADERS;SHADERS;LINK_LIBRARIES" "" ${ARGN})

  eq_stringify_shaders(SHADERS ${THIS_SHADERS})

  set(THIS_TARGET ${NAME})
  set(THIS_INPUT)
  source_group(\\ FILES ${THIS_SOURCES} ${THIS_HEADERS} ${THIS_SHADERS})
  add_executable(${THIS_TARGET} MACOSX_BUNDLE ${THIS_SOURCES} ${THIS_HEADERS} ${SHADERS})

  purple_expand_libraries(LINK_LIBRARIES ${THIS_LINK_LIBRARIES})
  target_link_libraries(${THIS_TARGET} ${LINK_LIBRARIES})

  set_target_properties(${THIS_TARGET} PROPERTIES OUTPUT_NAME ${NAME} FOLDER "Examples")
  add_dependencies(${THIS_TARGET} lib_EqualizerServer_shared) # dlopen'ed

  install(TARGETS ${THIS_TARGET} DESTINATION bin COMPONENT apps)
  install(FILES ${THIS_SOURCES} ${THIS_HEADERS} ${THIS_SHADERS}
    DESTINATION share/Equalizer/examples/${NAME} COMPONENT examples)
endfunction(EQ_ADD_EXAMPLE NAME)

include_directories(include)

install(DIRECTORY configs
  DESTINATION share/Equalizer
  COMPONENT data
  PATTERN ".svn" EXCLUDE
  )

install(DIRECTORY include
  DESTINATION share/Equalizer/examples
  COMPONENT examples
  PATTERN ".svn" EXCLUDE
  )

add_subdirectory(eqHello)
add_subdirectory(eqNBody)
add_subdirectory(eqPixelBench)
add_subdirectory(eqPly)
add_subdirectory(eVolve)
add_subdirectory(eqAsync)
add_subdirectory(osgScaleViewer)
