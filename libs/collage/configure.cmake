##
# Path : libs/collage/configure.cmake
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>
#               2010 Cedric Stalder <cedric.stalder@gmail.ch>
##

set(COLLAGE_DEFINES)

# is the LITTLE_ENDIAN macro actually used?
# maybe use BOOST_LITTLE_ENDIAN and BOOST_BIG_ENDIAN instead?
include(TestBigEndian)
test_big_endian(EQ_BIG_ENDIAN)
if(NOT EQ_BIG_ENDIAN)
  list(APPEND COLLAGE_DEFINES LITTLE_ENDIAN)
endif(NOT EQ_BIG_ENDIAN)

# if Boost is considered as a required dep, this macro should be obsolete
if(Boost_FOUND)
  list(APPEND COLLAGE_DEFINES EQ_USE_BOOST)
endif(Boost_FOUND)

if(CO_OPENMP_USED)
  list(APPEND COLLAGE_DEFINES EQ_USE_OPENMP)
endif(CO_OPENMP_USED)

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

set(DEFINES_FILE ${EQ_INCLUDE_DIR}/co/base/defines${ARCH}.h)
set(DEFINES_FILE_IN ${CMAKE_CURRENT_BINARY_DIR}/defines.h.in)

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

configure_file(${DEFINES_FILE_IN} ${DEFINES_FILE} COPYONLY)
install(FILES ${DEFINES_FILE} DESTINATION include/co/base/ COMPONENT dev)
