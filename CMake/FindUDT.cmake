
find_path(_udt_INCLUDE_DIR udt.h
  HINTS ${UDT_ROOT}/include
  PATHS /usr/include /usr/local/include /opt/local/include /usr/include/udt)

find_library(_udt_LIBRARY NAMES udt
  HINTS ${UDT_ROOT}/lib
  PATHS /usr/lib /usr/local/lib /opt/local/lib)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(UDT DEFAULT_MSG
  _udt_INCLUDE_DIR _udt_LIBRARY)

set(UDT_INCLUDE_DIRS ${_udt_INCLUDE_DIR})
set(UDT_LIBRARIES ${_udt_LIBRARY})
if(_udt_INCLUDE_DIR)
  file(STRINGS "${_udt_INCLUDE_DIR}/udt.h" UDT_HAS_RCVDATA REGEX UDT_RCVDATA)
endif()

if(UDT_FOUND AND NOT UDT_FIND_QUIETLY)
  message(STATUS "Found UDT in ${UDT_INCLUDE_DIRS};${UDT_LIBRARIES}")
endif()

