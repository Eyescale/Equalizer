# Copyright (c) 2011-2012 Stefan Eilemann <eile@eyescale.ch>
#
# - Find jsoncpp
# Find the jsonpp includes and library
# This module defines
#  JSONCPP_INCLUDE_DIR, where to find json.h, etc.
#  JSONCPP_FOUND, If false, do not try to use jsoncpp.
#  also defined, but not for general use are
#  JSONCPP_LIBRARY, where to find the jsoncpp library.
 
FIND_PATH(JSONCPP_INCLUDE_DIR jsoncpp/json/json.h
/usr/local/include
/usr/include
)
 
 
SET(JSONCPP_NAMES ${JSONCPP_NAMES} libjsoncpp.so)
FIND_LIBRARY(JSONCPP_LIBRARY
  NAMES ${JSONCPP_NAMES}
  PATHS /usr/lib /usr/local/lib
  )
 
IF (JSONCPP_LIBRARY AND JSONCPP_INCLUDE_DIR)
    SET(JSONCPP_LIBRARIES ${JSONCPP_LIBRARY})
    SET(JSONCPP_FOUND "YES")
ELSE (JSONCPP_LIBRARY AND JSONCPP_INCLUDE_DIR)
  SET(JSONCPP_FOUND "NO")
ENDIF (JSONCPP_LIBRARY AND JSONCPP_INCLUDE_DIR)
 
 
IF (JSONCPP_FOUND)
   IF (NOT JSONCPP_FIND_QUIETLY)
      MESSAGE(STATUS "Found jsoncpp: ${JSONCPP_LIBRARIES}")
   ENDIF (NOT JSONCPP_FIND_QUIETLY)
ELSE (JSONCPP_FOUND)
   IF (JSONCPP_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find jsoncpp library")
   ENDIF (JSONCPP_FIND_REQUIRED)
ENDIF (JSONCPP_FOUND)
 
MARK_AS_ADVANCED(
  JSONCPP_LIBRARY
  JSONCPP_INCLUDE_DIR
  )


