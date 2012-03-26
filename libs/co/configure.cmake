##
# Path : libs/collage/configure.cmake
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2012 Stefan Eilemann <eile@eyescale.ch>
#               2010 Cedric Stalder <cedric.stalder@gmail.ch>
##

if(NOT EQ_REVISION)
  set(EQ_REVISION 0)
endif()
update_file(${CMAKE_CURRENT_SOURCE_DIR}/version.in.h ${OUTPUT_INCLUDE_DIR}/co/version.h)
install(FILES ${OUTPUT_INCLUDE_DIR}/co/version.h DESTINATION include/co/ COMPONENT codev)

set(COLLAGE_DEFINES)

if(NOT EQ_BIG_ENDIAN)
  list(APPEND COLLAGE_DEFINES LITTLE_ENDIAN)
endif(NOT EQ_BIG_ENDIAN)

# if Boost is considered as a required dep, this macro should be obsolete
if(Boost_FOUND)
  list(APPEND COLLAGE_DEFINES CO_USE_BOOST)
  if(CO_USE_BOOST_SERIALIZATION)
    list(APPEND COLLAGE_DEFINES CO_USE_BOOST_SERIALIZATION)
  endif()
endif(Boost_FOUND)

if(CO_OPENMP_USED)
  list(APPEND COLLAGE_DEFINES CO_USE_OPENMP)
endif(CO_OPENMP_USED)

if(OFED_FOUND)
  list(APPEND COLLAGE_DEFINES CO_USE_OFED)
endif(OFED_FOUND)

if(UDT_FOUND)
  list(APPEND COLLAGE_DEFINES CO_USE_UDT)
endif(UDT_FOUND)

if(HWLOC_FOUND)
  list(APPEND COLLAGE_DEFINES CO_USE_HWLOC)
endif(HWLOC_FOUND)

if(COLLAGE_AGGRESSIVE_CACHING)
  list(APPEND COLLAGE_DEFINES CO_AGGRESSIVE_CACHING)
endif()

# maybe use BOOST_WINDOWS instead?
if(WIN32)
  list(APPEND COLLAGE_DEFINES
    WIN32
    WIN32_API
    WIN32_LEAN_AND_MEAN
    #EQ_INFINIBAND #Enable for IB builds (needs WinOF 2.0 installed)
    )
  set(ARCH Win32)
endif(WIN32)

if(APPLE)
  list(APPEND COLLAGE_DEFINES Darwin)
  set(ARCH Darwin)
endif(APPLE)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  list(APPEND COLLAGE_DEFINES Linux)
  set(ARCH Linux)
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

set(DEFINES_FILE ${OUTPUT_INCLUDE_DIR}/co/base/defines${ARCH}.h)
set(DEFINES_FILE_IN ${CMAKE_CURRENT_BINARY_DIR}/defines${ARCH}.h.in)

file(WRITE ${DEFINES_FILE_IN}
  "#ifndef COBASE_DEFINES_${ARCH}_H\n"
  "#define COBASE_DEFINES_${ARCH}_H\n\n"
  )

foreach(DEF ${COLLAGE_DEFINES})
  file(APPEND ${DEFINES_FILE_IN}
    "#ifndef ${DEF}\n"
    "#  define ${DEF}\n"
    "#endif\n"
    )
endforeach(DEF ${COLLAGE_DEFINES})

file(APPEND ${DEFINES_FILE_IN}
  "\n#endif /* COBASE_DEFINES_${ARCH}_H */\n"
  )

update_file(${DEFINES_FILE_IN} ${DEFINES_FILE})
install(FILES ${DEFINES_FILE} DESTINATION include/co/base/ COMPONENT codev)
