
# Copyright (c) 2013 Daniel Nachbaur <daniel.nachbaur@epfl.ch>

# This file creates package information for find_package by generating
# ${CMAKE_PROJECT_NAME}Config.cmake and ${CMAKE_PROJECT_NAME}ConfigVersion.cmake
# files. Those files are used in the config-mode of find_package which
# supersedes the Find${CMAKE_PROJECT_NAME}.cmake file.
#
# TODO
# - components

include(CMakePackageConfigHelpers)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake.in
  "set(${CMAKE_PROJECT_NAME}_VERSION ${VERSION})\n"
  "\n"
  "@PACKAGE_INIT@\n"
  "\n"
  "set(${UPPER_PROJECT_NAME}_LIBRARIES)\n"
  "set(${UPPER_PROJECT_NAME}_LIBRARY_NAMES "@LIBRARY_NAMES@")\n"
  "foreach(_libraryname \${${UPPER_PROJECT_NAME}_LIBRARY_NAMES})\n"
  "  find_library(\${_libraryname}_libraryname NAMES \${_libraryname} NO_DEFAULT_PATH\n"
  "               PATHS \${PACKAGE_PREFIX_DIR} PATH_SUFFIXES lib ${PYTHON_LIBRARY_PREFIX})\n"
  "  list(APPEND ${UPPER_PROJECT_NAME}_LIBRARIES \${\${_libraryname}_libraryname})\n"
  "endforeach()\n"
  "@TRANSIENTS@"
  "set_and_check(${UPPER_PROJECT_NAME}_INCLUDE_DIR "@PACKAGE_INCLUDE_INSTALL_DIR@")\n"
  "set(${UPPER_PROJECT_NAME}_DEB_DEPENDENCIES \"${LOWER_PROJECT_NAME}${VERSION_ABI}-lib (>= ${VERSION_MAJOR}.${VERSION_MINOR})\")\n"
  "\n"
  "if(EXISTS \${PACKAGE_PREFIX_DIR}/${CMAKE_MODULE_INSTALL_PATH}/options.cmake)\n"
  "  include(\${PACKAGE_PREFIX_DIR}/${CMAKE_MODULE_INSTALL_PATH}/options.cmake)\n"
  "endif()\n"
  "\n"
  "check_required_components(${CMAKE_PROJECT_NAME})\n"
)

set(INCLUDE_INSTALL_DIR include)

get_property(LIBRARY_TARGETS GLOBAL PROPERTY ALL_LIB_TARGETS)
set(LIBRARY_NAMES)
foreach(_target ${LIBRARY_TARGETS})
  get_target_property(_libraryname ${_target} OUTPUT_NAME)
  if(${_libraryname} MATCHES "_libraryname-NOTFOUND")
    set(_libraryname ${_target})
  else()
    get_target_property(_fullpath ${_target} LOCATION)
    get_filename_component(_suffix ${_fullpath} EXT)
    set(_libraryname "${_libraryname}${_suffix}")
  endif()
  list(APPEND LIBRARY_NAMES ${_libraryname})
endforeach()

set(TRANSIENTS "\n")
foreach(_transient ${${UPPER_PROJECT_NAME}_TRANSIENT_LIBRARIES})
  list(APPEND TRANSIENTS "find_package(${_transient} ${${${_transient}_name}_VERSION} EXACT REQUIRED)
list(APPEND ${UPPER_PROJECT_NAME}_LIBRARIES \${${${_transient}_name}_LIBRARIES})\n\n")
endforeach()
string(REGEX REPLACE ";" " " TRANSIENTS ${TRANSIENTS})

configure_package_config_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${CMAKE_MODULE_INSTALL_PATH}
  PATH_VARS INCLUDE_INSTALL_DIR LIBRARY_NAMES TRANSIENTS)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
  VERSION ${VERSION} COMPATIBILITY AnyNewerVersion)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
  DESTINATION ${CMAKE_MODULE_INSTALL_PATH})
