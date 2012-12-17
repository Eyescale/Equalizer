# _____________________________________________________________________________
#
# VRPN 
# _____________________________________________________________________________
# Assume not found.
SET(VRPN_FOUND FALSE)

# PATH ________________________________________________________________________

if (VRPN_PATH)
    # Set by user...
else (VRPN_PATH)
    find_path(VRPN_PATH include/vrpn_Tracker.h
	HINTS ${VRPN_ROOT} $ENV{VRPN_ROOT}
        /usr/local/
        /opt/VRPN
        "C:/Program Files/VRPN"
        /opt
        /sw/VRPN)
endif (VRPN_PATH)


if (VRPN_PATH)
    set (VRPN_FOUND TRUE)
endif (VRPN_PATH)

# HEADERS _____________________________________________________________________

if (VRPN_FOUND)
    set (VRPN_INCLUDE_DIRS ${VRPN_PATH}/include)
    mark_as_advanced (VRPN_INCLUDE_DIR)
endif (VRPN_FOUND)

# STATIC LIBRARY ______________________________________________________________

if (VRPN_FOUND)
    find_library(VRPN_LIBRARIES NAMES vrpn
	HINTS ${VRPN_ROOT}/lib $ENV{VRPN_ROOT}/lib
        PATHS ${VRPN_PATH}/lib
    )
    mark_as_advanced(VRPN_LIBRARIES)
endif (VRPN_FOUND)

# FOUND _______________________________________________________________________
if(VRPN_FOUND)
    message(STATUS
      "Found VRPN in ${VRPN_INCLUDE_DIRS};${VRPN_LIBRARIES}")
endif()

if (NOT VRPN_FOUND)
   if (VRPN_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find VRPN")
   endif (VRPN_FIND_REQUIRED)
endif (NOT VRPN_FOUND)
  

