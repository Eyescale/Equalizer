# Copyright (c) 2013-2015 Stefan.Eilemann@epfl.ch

include_directories(BEFORE ${PROJECT_SOURCE_DIR}/examples/include
  ${PROJECT_SOURCE_DIR}/examples)
add_definitions(-DEQ_SYSTEM_INCLUDES) # get GL headers
list(APPEND CPPCHECK_EXTRA_ARGS -I${PROJECT_SOURCE_DIR}/examples/include
  -I${PROJECT_SOURCE_DIR}/examples)

if(WIN32)
  set(AFFINITYCHECK_SOURCES affinityCheck/affinityCheck.cpp)
  set(AFFINITYCHECK_LINK_LIBRARIES ${GLEW_LIBRARY} ${OPENGL_gl_LIBRARY})
  common_application(affinityCheck)
endif()

set(EQTHREADAFFINITY_SOURCES threadAffinity/threadAffinity.cpp)
set(EQTHREADAFFINITY_LINK_LIBRARIES Equalizer EqualizerServer)
common_application(eqThreadAffinity)

set(EQPLYCONVERTER_SOURCES eqPlyConverter/main.cpp)
set(EQPLYCONVERTER_LINK_LIBRARIES Equalizer triply)
common_application(eqPlyConverter)

set(EQWINDOWADMIN_SOURCES windowAdmin/main.cpp)
set(EQWINDOWADMIN_LINK_LIBRARIES EqualizerAdmin)
common_application(eqWindowAdmin)

set(EQSERVER_SOURCES server/eqServer.cpp)
set(EQSERVER_LINK_LIBRARIES EqualizerServer)
common_application(eqServer)

list(APPEND CPPCHECK_EXTRA_ARGS --suppress=invalidscanf
  --suppress=invalidscanf_libc
  --suppress=variableScope --suppress=invalidPointerCast
  --suppress=invalidPrintfArgType_sint) # Yes, it's that bad.

set(EVOLVECONVERTER_HEADERS
  eVolveConverter/codebase.h
  eVolveConverter/ddsbase.h
  eVolveConverter/eVolveConverter.h
  eVolveConverter/hlp.h)
set(EVOLVECONVERTER_SOURCES
  eVolveConverter/eVolveConverter.cpp
  eVolveConverter/ddsbase.cpp)
set(EVOLVECONVERTER_LINK_LIBRARIES ${Boost_PROGRAM_OPTIONS_LIBRARY})
add_definitions(-DBOOST_PROGRAM_OPTIONS_DYN_LINK)
common_application(eVolveConverter)
