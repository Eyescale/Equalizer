# Copyright (c) 2010 Daniel Pfeifer

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/examples/include # test depend on examples... oh well
  )

file(GLOB_RECURSE TEST_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

set(ALL_TESTS)
foreach(FILE ${TEST_FILES})
  string(REGEX REPLACE "[./]" "_" NAME ${FILE})
  add_executable(test_${NAME} EXCLUDE_FROM_ALL ${FILE})
  target_link_libraries(test_${NAME} Equalizer EqualizerServer)
  get_target_property(EXECUTABLE test_${NAME} LOCATION)
  add_test(${NAME} ${EXECUTABLE})
  list(APPEND ALL_TESTS test_${NAME})
endforeach(FILE ${TEST_FILES})

# workaround bug that tests won't get built
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS ${ALL_TESTS})
