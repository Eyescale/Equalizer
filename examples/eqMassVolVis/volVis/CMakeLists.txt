
# Copyright (c) 2011 Maxim Makhinya <maxmah@gmail.com>
#               2012 David Steiner  <steiner@ifi.uzh.ch>


if(MSVC)
  add_definitions(/wd4996)
endif(MSVC)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


# if(CUDA_FOUND)
#   include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})
#   message(STATUS "Using CUDA from: ${CUDA_INCLUDE_DIRS}")
# endif()

include(files.cmake)

set(MSV_LIB "${PROJECT_BINARY_DIR}/lib/libmsv.so")

source_group(volVis REGULAR_EXPRESSION .*)
source_group(\\ FILES CMakeLists.txt files.cmake ${VOLVIS_MAIN_SOURCES} )
source_group(asyncFetcher   FILES ${VOLVIS_ASYNC_FETCHER_HEADERS}   ${VOLVIS_ASYNC_FETCHER_SOURCES} )
source_group(EQ             FILES ${VOLVIS_EQ_HEADERS}              ${VOLVIS_EQ_SOURCES}            )
source_group(LB             FILES ${VOLVIS_LB_HEADERS}              ${VOLVIS_LB_SOURCES}            )
source_group(render         FILES ${VOLVIS_RENDERER_HEADERS}        ${VOLVIS_RENDERER_SOURCES}      )
source_group(util           FILES ${VOLVIS_UTIL_HEADERS} )
source_group(shaders        FILES ${VOLVIS_SHADERS} )

eq_add_example(volVis
  HEADERS
    ${VOLVIS_ASYNC_FETCHER_HEADERS}
    ${VOLVIS_EQ_HEADERS}
    ${VOLVIS_LB_HEADERS}
    ${VOLVIS_RENDERER_HEADERS}
    ${VOLVIS_UTIL_HEADERS}
  SOURCES
    ${VOLVIS_ASYNC_FETCHER_SOURCES}
    ${VOLVIS_EQ_SOURCES}
    ${VOLVIS_LB_SOURCES}
    ${VOLVIS_RENDERER_SOURCES}
    ${VOLVIS_MAIN_SOURCES}
  SHADERS
    ${VOLVIS_SHADERS}
  LINK_LIBRARIES
    ${CARBON_LIBRARY}
#    ${CUDA_LIBRARIES}
    ${LUNCHBOX_LIBRARIES}
    ${MSV_LIB}
    X11
  )



source_group(_volVisTests REGULAR_EXPRESSION .*)
source_group(\\ FILES ${VOLVIS_MAIN_TESTS_SOURCES} )

eq_add_example(_volVisTests
  HEADERS
    ${VOLVIS_ASYNC_FETCHER_HEADERS}
    ${VOLVIS_EQ_HEADERS}
    ${VOLVIS_LB_HEADERS}
    ${VOLVIS_RENDERER_HEADERS}
    ${VOLVIS_UTIL_HEADERS}
  SOURCES
    ${VOLVIS_ASYNC_FETCHER_SOURCES}
    ${VOLVIS_EQ_SOURCES}
    ${VOLVIS_LB_SOURCES}
    ${VOLVIS_RENDERER_SOURCES}
    ${VOLVIS_MAIN_TESTS_SOURCES}
  SHADERS
    ${VOLVIS_SHADERS}
  LINK_LIBRARIES
    ${CARBON_LIBRARY}
#    ${CUDA_LIBRARIES}
    ${LUNCHBOX_LIBRARIES}
    ${MSV_LIB}
    X11
  )

