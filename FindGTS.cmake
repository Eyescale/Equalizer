# Try to find gnu triangulation library GTS
# See
# http://gts.sf.net
#
# Once run this will define:
#
# GTS_FOUND       = system has GTS lib
#
# GTS_LIBRARIES   = full path to the libraries
#    on Unix/Linux with additional linker flags from "gts-config --libs"
#
# CMAKE_GTS_CXX_FLAGS  = Unix compiler flags for GTS, essentially "`gts-config --cxxflags`"
#
# GTS_INCLUDE_DIR      = where to find headers
#
# GTS_LINK_DIRECTORIES = link directories, useful for rpath on Unix
# GTS_EXE_LINKER_FLAGS = rpath on Unix
#
#
# GTS modyfication by
# A.Khalatyan 05/2009
#
# www.aip.de/~arm2arm
#
# This script is based on GSL script by
# Felix Woelk 07/2004
# and  Jan Woetzel
# www.mip.informatik.uni-kiel.de
# --------------------------------

if(GTS_FIND_REQUIRED)
  set(_gts_output_type FATAL_ERROR)
  set(_gts_output 1)
else()
  set(_gts_output_type STATUS)
  if(NOT GTS_FIND_QUIETLY)
    set(_gts_output 1)
  endif()
endif()

IF(WIN32)
  # JW tested with gsl-1.8, Windows XP, MSVS 7.1, MSVS 8.0
  SET(GTS_POSSIBLE_ROOT_DIRS
    ${GTS_ROOT_DIR}
    $ENV{GTS_ROOT_DIR}
    ${GTS_DIR}
    ${GTS_HOME}
    $ENV{GTS_DIR}
    $ENV{GTS_HOME}
    $ENV{EXTERN_LIBS_DIR}/gts
    $ENV{EXTRA}
    # "C:/home/arm2arm/SOFTWARE/gts-0.7.6"
    )
  FIND_PATH(GTS_INCLUDE_DIR
    NAMES gts.h gtsconfig.h
    PATHS ${GTS_POSSIBLE_ROOT_DIRS}
    PATH_SUFFIXES include
    DOC "GTS header include dir"
    )

  FIND_LIBRARY(GTS_GTS_LIBRARY
    NAMES gts libgts
    PATHS  ${GTS_POSSIBLE_ROOT_DIRS}
    PATH_SUFFIXES lib
    DOC "GTS library dir" )

#  FIND_LIBRARY(GTS_GLIB_LIBRARY
#    NAMES glib libgslcblas
#    PATHS  ${GSL_POSSIBLE_ROOT_DIRS}
#    PATH_SUFFIXES lib
#    DOC "GSL cblas library dir" )

  SET(GTS_LIBRARIES ${GTS_GTS_LIBRARY})

  #MESSAGE("DBG\n"
    #  "GSL_GSL_LIBRARY=${GSL_GSL_LIBRARY}\n"
    #  "GSL_GSLCBLAS_LIBRARY=${GSL_GSLCBLAS_LIBRARY}\n"
    #  "GSL_LIBRARIES=${GSL_LIBRARIES}")


ELSE(WIN32)

  IF(UNIX)
    SET(GSL_CONFIG_PREFER_PATH
      "$ENV{GTS_DIR}/bin"
      "$ENV{GTS_DIR}"
      "$ENV{GTS_HOME}/bin"
      "$ENV{GTS_HOME}"
      CACHE STRING "preferred path to GTS (gts-config)")
    FIND_PROGRAM(GTS_CONFIG gts-config
      ${GTS_CONFIG_PREFER_PATH}
      /work2/arm2arm/SOFTWARE/bin/
      )
    #MESSAGE("DBG GTS_CONFIG ${GTS_CONFIG}")

    IF (GTS_CONFIG)
      if(_gts_output)
        MESSAGE(STATUS "GTS using gts-config ${GTS_CONFIG}")
      endif()
      # set CXXFLAGS to be fed into CXX_FLAGS by the user:
      EXEC_PROGRAM(${GTS_CONFIG}
        ARGS --cflags
        OUTPUT_VARIABLE  GTS_CXX_FLAGS )
       #SET(GTS_CXX_FLAGS "`${GTS_CONFIG} --cflags`")
      SET(CMAKE_GTS_CXX_FLAGS ${GTS_CXX_FLAGS})
      #      MESSAGE("DBG ${GTS_CXX_FLAGS}")
      # set INCLUDE_DIRS to prefix+include
      EXEC_PROGRAM(${GTS_CONFIG}
        ARGS --prefix
        OUTPUT_VARIABLE GTS_PREFIX)
      SET(GTS_INCLUDE_DIR ${GTS_PREFIX}/include CACHE STRING INTERNAL)
      # set link libraries and link flags

      #SET(GSL_LIBRARIES "`${GSL_CONFIG} --libs`")

      # extract link dirs for rpath
      EXEC_PROGRAM(${GTS_CONFIG}
        ARGS --libs
        OUTPUT_VARIABLE  GTS_CONFIG_LIBS )
      SET(GTS_LIBRARIES "${GTS_CONFIG_LIBS}")

      # split off the link dirs (for rpath)
      # use regular expression to match wildcard equivalent "-L*<endchar>"
      # with <endchar> is a space or a semicolon
      STRING(REGEX MATCHALL "[-][L]([^ ;])+"
        GTS_LINK_DIRECTORIES_WITH_PREFIX
        "${GTS_CONFIG_LIBS}" )
      #      MESSAGE("DBG  GSL_LINK_DIRECTORIES_WITH_PREFIX=${GSL_LINK_DIRECTORIES_WITH_PREFIX}")

      # remove prefix -L because we need the pure directory for LINK_DIRECTORIES

      IF (GTS_LINK_DIRECTORIES_WITH_PREFIX)
        STRING(REGEX REPLACE "[-][L]" "" GTS_LINK_DIRECTORIES ${GTS_LINK_DIRECTORIES_WITH_PREFIX} )
      ENDIF (GTS_LINK_DIRECTORIES_WITH_PREFIX)
      SET(GTS_EXE_LINKER_FLAGS "-Wl,-rpath,${GTS_LINK_DIRECTORIES}" CACHE STRING INTERNAL)
      #      MESSAGE("DBG  GSL_LINK_DIRECTORIES=${GSL_LINK_DIRECTORIES}")
      #      MESSAGE("DBG  GSL_EXE_LINKER_FLAGS=${GSL_EXE_LINKER_FLAGS}")

      #      ADD_DEFINITIONS("-DHAVE_GSL")
      #      SET(GSL_DEFINITIONS "-DHAVE_GSL")
      MARK_AS_ADVANCED(
        GTS_CXX_FLAGS
        GTS_INCLUDE_DIR
        GTS_LIBRARIES
        GTS_LINK_DIRECTORIES
        GTS_DEFINITIONS
	)
      if(_gts_output)
        MESSAGE(STATUS "Using GTS from ${GTS_PREFIX}")
      endif()
    ELSE(GTS_CONFIG)

      INCLUDE(UsePkgConfig) #needed for PKGCONFIG(...)

      if(_gts_output)
        MESSAGE(STATUS "GSL using pkgconfig")
      endif()
      #      PKGCONFIG(gsl includedir libdir linkflags cflags)
      PKGCONFIG(gts GTS_INCLUDE_DIR GTS_LINK_DIRECTORIES GTS_LIBRARIES GTS_CXX_FLAGS)
      IF(GTS_INCLUDE_DIR)
	MARK_AS_ADVANCED(
          GTS_CXX_FLAGS
          GTS_INCLUDE_DIR
          GTS_LIBRARIES
          GTS_LINK_DIRECTORIES
	  )

      ELSEIF(_gts_output)
	MESSAGE(${_gts_output_type}
          "FindGTS.cmake: gts-config/pkg-config gts not found. Please set it manually. GTS_CONFIG=${GTS_CONFIG}")
      ENDIF()

    ENDIF(GTS_CONFIG)

  ENDIF(UNIX)
ENDIF(WIN32)


IF(GTS_LIBRARIES)
  IF(GTS_INCLUDE_DIR OR GTS_CXX_FLAGS)

    SET(GTS_FOUND 1)

  ENDIF(GTS_INCLUDE_DIR OR GTS_CXX_FLAGS)
ENDIF(GTS_LIBRARIES)


# ==========================================
IF(NOT GTS_FOUND AND _gts_output)
  MESSAGE(${_gts_output_type} "GTS was not found.")
ENDIF()
