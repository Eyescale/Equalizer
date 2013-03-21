
# Copyright (c) 2011 Maxim Makhinya <maxmah@gmail.com>
#               2012 David Steiner  <steiner@ifi.uzh.ch>


include(files.cmake)

#if(Boost_FOUND)
#  list(APPEND CO_ADD_LINKLIB ${Boost_SYSTEM_LIBRARY})
#  if(NOT Boost_USE_STATIC_LIBS)
#    list(APPEND CO_ADD_LINKLIB ${Boost_DATE_TIME_LIBRARY})
#  endif()
#endif(Boost_FOUND)

# XXX: maybe check for compiler...
STRING(REGEX REPLACE "-Werror" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )
STRING(REGEX REPLACE "/WX" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )

source_group(msv REGULAR_EXPRESSION .*)
source_group(\\ FILES CMakeLists.txt files.cmake)
source_group(IO     FILES ${MSV_IO_PUBLIC_HEADERS}    ${MSV_IO_SOURCES}    )
source_group(tree   FILES ${MSV_TREE_PUBLIC_HEADERS}  ${MSV_TREE_SOURCES}  )
source_group(types  FILES ${MSV_TYPES_PUBLIC_HEADERS} ${MSV_TYPES_SOURCES} )
source_group(util   FILES ${MSV_UTIL_PUBLIC_HEADERS}  ${MSV_UTIL_SOURCES}  )

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

set(MSV_ALL_SOURCES
  ${MSV_IO_SOURCES}
  ${MSV_TREE_SOURCES}
  ${MSV_TYPES_SOURCES}
  ${MSV_UTIL_SOURCES}
  )

set(MSV_ALL_HEADERS
  ${MSV_IO_PUBLIC_HEADERS}
  ${MSV_TREE_PUBLIC_HEADERS}
  ${MSV_TYPES_PUBLIC_HEADERS}
  ${MSV_UTIL_PUBLIC_HEADERS}
  )

purple_add_library(msv SHARED
  HEADERS_PREFIX "msv/"
  HEADERS ${MSV_ALL_HEADERS}
  SOURCES ${MSV_ALL_SOURCES}
  LINK_LIBRARIES
    "png"
    ${LUNCHBOX_LIBRARIES}
  )

