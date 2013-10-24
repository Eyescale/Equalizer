# - Try to find libjpeg-turbo
# Once done, this will define
#
#  LibJpegTurbo_FOUND - system has libjpeg-turbo
#  LibJpegTurbo_INCLUDE_DIRS - the libjpeg-turbo include directories
#  LibJpegTurbo_LIBRARIES - link these to use libjpeg-turbo
#
# this file is modeled after http://www.cmake.org/Wiki/CMake:How_To_Find_Libraries

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(LibJpegTurbo_PKGCONF LibJpegTurbo)

# Include dir
find_path(LibJpegTurbo_INCLUDE_DIR
  NAMES turbojpeg.h
  PATHS ${LibJpegTurbo_PKGCONF_INCLUDE_DIRS} /opt/libjpeg-turbo/include
)

# Finally the library itself
find_library(LibJpegTurbo_LIBRARY
  NAMES turbojpeg.so
  PATHS ${LibJpegTurbo_PKGCONF_LIBRARY_DIRS} /opt/libjpeg-turbo/lib
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(LibJpegTurbo_PROCESS_INCLUDES LibJpegTurbo_INCLUDE_DIR)
set(LibJpegTurbo_PROCESS_LIBS LibJpegTurbo_LIBRARY)
libfind_process(LibJpegTurbo)
