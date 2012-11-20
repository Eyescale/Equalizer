# _____________________________________________________________________________
#
# WIIUSE 
# _____________________________________________________________________________

# Assume not found.
SET(WIIUSE_FOUND FALSE)

# PATH ________________________________________________________________________

if (WIIUSE_PATH)
    # Set by user...
else (WIIUSE_PATH)
    find_path(WIIUSE_PATH include/wiiuse.h
	HINTS ${WIIUSE_ROOT} $ENV{WIIUSE_ROOT}
        /usr/local/
        /usr/
    )
endif (WIIUSE_PATH)

if (WIIUSE_PATH)
    set (WIIUSE_FOUND TRUE)
endif (WIIUSE_PATH)

# HEADERS _____________________________________________________________________

if (WIIUSE_FOUND)
    set (WIIUSE_INCLUDE_DIR ${WIIUSE_PATH}/include)
    mark_as_advanced (WIIUSE_INCLUDE_DIR)
endif (WIIUSE_FOUND)

# STATIC LIBRARY ______________________________________________________________

if (WIIUSE_FOUND)
    find_library(WIIUSE_LIBRARIES NAMES wiiuse
	HINTS ${WIIUSE_ROOT} $ENV{WIIUSE_ROOT}
        PATHS ${WIIUSE_PATH}/lib
    )
    mark_as_advanced(WIIUSE_LIBRARIES)
endif (WIIUSE_FOUND)

# FOUND _______________________________________________________________________
if(WIIUSE_FOUND)
    message(STATUS
      "Found WiiUse in ${WIIUSE_INCLUDE_DIR};${WIIUSE_LIBRARIES}")
endif()

if (NOT WIIUSE_FOUND)
   if (WIIUSE_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find Wiiuse")
   endif (WIIUSE_FIND_REQUIRED)
endif (NOT WIIUSE_FOUND)
  

