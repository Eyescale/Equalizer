
# Copyright (c) 2010-2014, Stefan Eilemann <eile@eyescale.ch>
#
# Change this number when adding tests to force a CMake run: 5

file(GLOB COMPOSITOR_IMAGES compositor/*.rgb)
file(COPY compressor/images ${PROJECT_SOURCE_DIR}/examples/configs
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
