# Copyright (c) 2010 Daniel Pfeifer
#               2010-2011, Stefan Eilemann <eile@eyescale.ch>

if(NOT EQUALIZER_BUILD_TESTS)
  return()
endif(NOT EQUALIZER_BUILD_TESTS)

if(NOT WIN32) # tests want to be with DLLs on Windows - no rpath
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Equalizer_BINARY_DIR}/tests)
endif()

file(GLOB COMPOSITOR_IMAGES eq/compositor/*.rgb)
file(GLOB TEST_CONFIGS eq/server/reliability/*.eqc)
file(COPY eq/compressor/images ${CMAKE_SOURCE_DIR}/examples/configs
	  ${COMPOSITOR_IMAGES} ${TEST_CONFIGS}
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/examples/include # test depend on examples... oh well
  ${CMAKE_SOURCE_DIR} # some tests need private headers
  )

file(GLOB_RECURSE TEST_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)

set(ALL_TESTS)
foreach(FILE ${TEST_FILES})
  set(THIS_BUILD ON)
  if(EQ_BIG_ENDIAN)
    if( ${FILE} MATCHES "eq/compressor/image.cpp" OR
        ${FILE} MATCHES "eq/compositor/test.cpp" )
      set(THIS_BUILD OFF)
    endif()
  endif()

  if(THIS_BUILD)
    string(REGEX REPLACE "[./]" "_" NAME ${FILE})
    source_group(\\ FILES ${FILE})
    add_executable(${NAME} ${FILE})
    if(${NAME} MATCHES "co_.*")
      target_link_libraries(${NAME} Collage)
    endif()
    if(${NAME} MATCHES "eq_.*")
      target_link_libraries(${NAME} lib_Equalizer_shared)
    endif()
    if(${NAME} MATCHES "eq_server_.*")
      target_link_libraries(${NAME} lib_EqualizerServer_shared)
    endif()
    if(${NAME} MATCHES "eq_fabric_.*")
      target_link_libraries(${NAME} lib_EqualizerFabric_shared)
    endif()
    get_target_property(EXECUTABLE ${NAME} LOCATION)
    STRING(REGEX REPLACE "\\$\\(.*\\)" "\${CTEST_CONFIGURATION_TYPE}"
      EXECUTABLE "${EXECUTABLE}")
    add_test(${NAME} ${EXECUTABLE})
    set_target_properties(${NAME} PROPERTIES FOLDER "Tests")
    list(APPEND ALL_TESTS ${NAME})
  endif()
endforeach(FILE ${TEST_FILES})

# workaround bug that tests won't get built
add_custom_target(tests DEPENDS ${ALL_TESTS})
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS tests)
set_target_properties(tests PROPERTIES FOLDER "Tests")
set_target_properties(check PROPERTIES FOLDER "Tests")
