# - Try to find TUIO
# Once done, this will define
#
#  TUIO_FOUND - system has TUIO
#  TUIO_INCLUDE_DIRS - the TUIO include directories
#  TUIO_LIBRARIES - link these to use TUIO
#
# this file is modeled after http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(TUIO_PKGCONF TUIO)

# Include dir
find_path(TUIO_INCLUDE_DIR
  NAMES TUIO/TuioClient.h
  PATHS ${TUIO_PKGCONF_INCLUDE_DIRS} ${TUIO_ROOT} $ENV{TUIO_ROOT}
)

# also add the TUIO/ directory to the include path
SET(TUIO_INCLUDE_DIR_2 "${TUIO_INCLUDE_DIR}/TUIO")

# also add the oscpack/ directory to the include path
SET(TUIO_INCLUDE_DIR_3 "${TUIO_INCLUDE_DIR}/oscpack")

# Finally the library itself
find_library(TUIO_LIBRARY
  NAMES TUIO
  PATHS ${TUIO_PKGCONF_LIBRARY_DIRS} ${TUIO_ROOT} $ENV{TUIO_ROOT}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(TUIO_PROCESS_INCLUDES TUIO_INCLUDE_DIR TUIO_INCLUDE_DIR_2 TUIO_INCLUDE_DIR_3)
set(TUIO_PROCESS_LIBS TUIO_LIBRARY)
libfind_process(TUIO)
