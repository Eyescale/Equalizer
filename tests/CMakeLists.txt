# Copyright (c) 2010 Daniel Pfeifer
#               2010-2011, Stefan Eilemann <eile@eyescale.ch>

option(EQUALIZER_BUILD_TESTS "Build Equalizer unit tests." ON)
if(NOT EQUALIZER_BUILD_TESTS)
  return()
endif(NOT EQUALIZER_BUILD_TESTS)

file(GLOB COMPOSITOR_IMAGES compositor/*.rgb)
file(GLOB TEST_CONFIGS reliability/*.eqc)
file(COPY image/images ${COMPOSITOR_IMAGES} ${TEST_CONFIGS}
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/examples/include # test depend on examples... oh well
  ${CMAKE_SOURCE_DIR} # some tests need private headers
  )

file(GLOB_RECURSE TEST_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

set(ALL_TESTS)
foreach(FILE ${TEST_FILES})
  string(REGEX REPLACE "[./]" "_" NAME ${FILE})
  source_group(\\ FILES ${FILE})
  add_executable(test_${NAME} ${FILE})
  target_link_libraries(test_${NAME} Equalizer lib_EqualizerServer_shared)
  get_target_property(EXECUTABLE test_${NAME} LOCATION)
  STRING(REGEX REPLACE "\\$\\(.*\\)" "\${CTEST_CONFIGURATION_TYPE}"
       EXECUTABLE "${EXECUTABLE}")
  add_test(${NAME} ${EXECUTABLE})
  set_target_properties(test_${NAME} PROPERTIES FOLDER "Tests")
  list(APPEND ALL_TESTS test_${NAME})
endforeach(FILE ${TEST_FILES})

# workaround bug that tests won't get built
add_custom_target(tests DEPENDS ${ALL_TESTS})
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS tests)
set_target_properties(tests PROPERTIES FOLDER "Tests")
set_target_properties(check PROPERTIES FOLDER "Tests")
