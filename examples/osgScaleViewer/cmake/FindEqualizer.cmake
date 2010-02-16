# FindEqualizer sets the following vars:
# Equalizer_FOUND
# Equalizer_INCLUDE_DIR
# Equalizer_LIBRARIES

# Look for Equalizer includes by searching for eq.h
FIND_PATH(Equalizer_INCLUDE_DIR eq/eq.h
  PATH_SUFFIXES include
  PATHS
  /usr/local
  /usr
  /opt
  ../../build/${CMAKE_SYSTEM_NAME}
  $ENV{EQ_ROOT}
  "C:/Programme/Equalizer/include"
  "C:/Program Files/Equalizer/include"
)

# Search for the Equalizer library by searching eq.so / Equalizer.lib
FIND_LIBRARY(Equalizer_LIBRARY
  NAMES eq Equalizer
  PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64
  PATHS
  /usr/local
  /usr
  /opt
  ../../build/${CMAKE_SYSTEM_NAME}
  $ENV{EQ_ROOT}
  "C:/Programme/Equalizer/lib"
  "C:/Program Files/Equalizer/lib"
)

# The variable Equalizer_LIBRARIES will contain all libraries which have to be
# linked when using Equalizer.
SET(Equalizer_LIBRARIES ${Equalizer_LIBRARY})

# Equalizer needs librt for clock_gettime, but only on Linux.
# Therefore search for librt and add it to Equalizer_LIBRARIES
IF("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  FIND_LIBRARY(RT_LIBRARY NAMES rt)
  IF(NOT RT_LIBRARY)
    MESSAGE(FATAL_ERROR "librt was NOT found!")
  ELSE(NOT RT_LIBRARY)
    MESSAGE(STATUS "Found librt: Library at ${RT_LIBRARY}")
  ENDIF(NOT RT_LIBRARY)
  SET(Equalizer_LIBRARIES ${Equalizer_LIBRARIES} ${RT_LIBRARY})
ENDIF("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")

IF("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  SET(CMAKE_CXX_FLAGS "-arch i386 ${CMAKE_CXX_FLAGS}")
  SET(CMAKE_C_FLAGS "-arch i386 ${CMAKE_CXX_FLAGS}")
ENDIF("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")

# This apparently is needed, because otherwise the compiler fails with
# things like "undefined reference to '__sync_sub_and_fetch_4".
# See also http://n2.nabble.com/Build-Issue-with-0.6-td1555631.html.
IF(UNIX AND CMAKE_SIZEOF_VOID_P EQUAL 4)
  #message("Detected UNIX with 32 bit.")
  SET(CMAKE_CXX_FLAGS "-msse3 -march=i686 -mtune=i686 ${CMAKE_CXX_FLAGS}")
  SET(CMAKE_C_FLAGS "-msse3 -march=686 -mcpu=i686 ${CMAKE_CXX_FLAGS}")
ENDIF(UNIX AND CMAKE_SIZEOF_VOID_P EQUAL 4)

# Set Equalizer_FOUND to YES if the includes and the libraries have been found.
SET(Equalizer_FOUND "NO")
IF(Equalizer_LIBRARIES AND Equalizer_INCLUDE_DIR)
  SET(Equalizer_FOUND "YES")
ENDIF(Equalizer_LIBRARIES AND Equalizer_INCLUDE_DIR)

