
# Copyright (c) 2013 Daniel Nachbaur <daniel.nachbaur@epfl.ch>

# This file creates package information for find_package by generating
# ${CMAKE_PROJECT_NAME}Config.cmake and ${CMAKE_PROJECT_NAME}ConfigVersion.cmake
# files. Those files are used in the config-mode of find_package which
# supersedes the Find${CMAKE_PROJECT_NAME}.cmake file.
#
# Input variables
#    ${UPPER_PROJECT_NAME}_TRANSIENT_LIBRARIES - A list of dependent link libraries, format is ${CMAKE_PROJECT_NAME}
#
# Output variables
#    ${UPPER_PROJECT_NAME}_FOUND - Was the project and all of the specified components found?
#
#    ${UPPER_PROJECT_NAME}_VERSION - The version of the project which was found
#
#    ${UPPER_PROJECT_NAME}_INCLUDE_DIRS - Where to find the headers
#
#    ${UPPER_PROJECT_NAME}_LIBRARIES - The project link libraries
#
#    ${UPPER_PROJECT_NAME}_LIBRARY - The produced (core) library
#
#    ${UPPER_PROJECT_NAME}_COMPONENTS - A list of components found
#
#    ${UPPER_PROJECT_NAME}_${component}_LIBRARY - The path & name of the ${component} library
#
#    ${UPPER_PROJECT_NAME}_DEB_DEPENDENCIES - A list of dependencies for the CPack deb generator


include(CMakePackageConfigHelpers)

# Write the ProjectConfig.cmake.in file for configure_package_config_file
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake.in
  "\n"
# add helper stuff from CMakePackageConfigHelpers
  "@PACKAGE_INIT@\n"
  "\n"
# add transient library finding
  "@TRANSIENTS@"
  "if(_fail)\n"
  "  set(${CMAKE_PROJECT_NAME}_FOUND)\n"
  "else()\n"
# setup VERSION, INCLUDE_DIRS and DEB_DEPENDENCIES
  "  set(${CMAKE_PROJECT_NAME}_VERSION ${VERSION})\n"
  "  set_and_check(${UPPER_PROJECT_NAME}_INCLUDE_DIRS "@PACKAGE_INCLUDE_INSTALL_DIR@")\n"
  "  set(${UPPER_PROJECT_NAME}_DEB_DEPENDENCIES \"${LOWER_PROJECT_NAME}${VERSION_ABI} (>= ${VERSION_MAJOR}.${VERSION_MINOR})\")\n"
  "\n"
# find the core library
  "  find_library(${UPPER_PROJECT_NAME}_LIBRARY ${CMAKE_PROJECT_NAME} NO_DEFAULT_PATH\n"
  "        PATHS \${PACKAGE_PREFIX_DIR} PATH_SUFFIXES lib ${PYTHON_LIBRARY_PREFIX})\n"
  "  if(${UPPER_PROJECT_NAME}_LIBRARY MATCHES "${UPPER_PROJECT_NAME}_LIBRARY-NOTFOUND")\n"
  "    set(${CMAKE_PROJECT_NAME}_FOUND)\n"
  "    if(_out)\n"
  "      message(\${_output_type} \"   Missing the ${CMAKE_PROJECT_NAME} \"\n"
  "        \"library in \${PACKAGE_PREFIX_DIR}/lib.\")\n"
  "    endif()\n"
  "  endif()\n"
# find components if specified
  "  if(${CMAKE_PROJECT_NAME}_FIND_COMPONENTS)\n"
  "    list(APPEND ${UPPER_PROJECT_NAME}_LIBRARIES \${${UPPER_PROJECT_NAME}_LIBRARY})\n"
  "    foreach(_component \${${CMAKE_PROJECT_NAME}_FIND_COMPONENTS})\n"
  "      find_library(\${_component}_libraryname ${CMAKE_PROJECT_NAME}_\${_component} NO_DEFAULT_PATH\n"
  "        PATHS \${PACKAGE_PREFIX_DIR} PATH_SUFFIXES lib ${PYTHON_LIBRARY_PREFIX})\n"
  "\n"
  "      if(\${_component}_libraryname MATCHES "\${_component}_libraryname-NOTFOUND")\n"
  "        set(${CMAKE_PROJECT_NAME}_\${_component}_FOUND)\n"
  "        if(_out)\n"
  "          message(\${_output_type} \"   ${CMAKE_PROJECT_NAME}_\${_component} \"\n"
  "            \"not found in \${PACKAGE_PREFIX_DIR}/lib\")\n"
  "        endif()\n"
  "      else()\n"
  "        set(${CMAKE_PROJECT_NAME}_\${_component}_FOUND TRUE)\n"
  "        set(${UPPER_PROJECT_NAME}_\${_component}_LIBRARY \${\${_component}_libraryname})\n"
  "        list(APPEND ${UPPER_PROJECT_NAME}_LIBRARIES \${\${_component}_libraryname})\n"
  "        list(APPEND ${UPPER_PROJECT_NAME}_COMPONENTS \${_component})\n"
  "      endif()\n"
  "    endforeach()\n"
  "  else()\n"
# if no component was specified, find all produced libraries
  "    set(${UPPER_PROJECT_NAME}_LIBRARY_NAMES "@LIBRARY_NAMES@")\n"
  "    foreach(_libraryname \${${UPPER_PROJECT_NAME}_LIBRARY_NAMES})\n"
  "      find_library(\${_libraryname}_LIBRARY \${_libraryname} NO_DEFAULT_PATH\n"
  "                   PATHS \${PACKAGE_PREFIX_DIR} PATH_SUFFIXES lib ${PYTHON_LIBRARY_PREFIX})\n"
  "      list(APPEND ${UPPER_PROJECT_NAME}_LIBRARIES \${\${_libraryname}_LIBRARY})\n"
  "    endforeach()\n"
  "  endif()\n"
  "\n"
# include options.cmake if existing
  "  if(EXISTS \${PACKAGE_PREFIX_DIR}/${CMAKE_MODULE_INSTALL_PATH}/options.cmake)\n"
  "    include(\${PACKAGE_PREFIX_DIR}/${CMAKE_MODULE_INSTALL_PATH}/options.cmake)\n"
  "  endif()\n"
  "\n"
  "  set(${CMAKE_PROJECT_NAME}_FOUND TRUE)\n"
  "  check_required_components(${CMAKE_PROJECT_NAME})\n"
  "endif()\n"
  "\n"
# finally report about found or not found
  "if(NOT ${CMAKE_PROJECT_NAME}_FOUND)\n"
  "  set(${CMAKE_PROJECT_NAME}_VERSION)\n"
  "  set(${UPPER_PROJECT_NAME}_INCLUDE_DIRS)\n"
  "  set(${UPPER_PROJECT_NAME}_LIBRARIES)\n"
  "  set(${UPPER_PROJECT_NAME}_DEB_DEPENDENCIES)\n"
  "  set(${UPPER_PROJECT_NAME}_LIBRARY)\n"
  "  set(${UPPER_PROJECT_NAME}_COMPONENTS)\n"
  "else()\n"
  "  if(_out)\n"
  "    set(${UPPER_PROJECT_NAME}_FOUND TRUE)\n"
  "    message(STATUS \"Found ${CMAKE_PROJECT_NAME} ${VERSION} in \"\n"
  "      \"\${${UPPER_PROJECT_NAME}_INCLUDE_DIRS}:\${${UPPER_PROJECT_NAME}_LIBRARY}\")\n"
  "  endif()\n"
  "endif()\n"
)

# location of the includes
set(INCLUDE_INSTALL_DIR include)

# compile the list of generated libraries
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

# compile finding of transient libraries
set(TRANSIENTS
  "if(${CMAKE_PROJECT_NAME}_FIND_REQUIRED)\n"
  "  set(_output_type FATAL_ERROR)\n"
  "  set(_out 1)\n"
  "  set(_req REQUIRED)\n"
  "else()\n"
  "  set(_output_type STATUS)\n"
  "  if(NOT ${CMAKE_PROJECT_NAME}_FIND_QUIETLY)\n"
  "    set(_out 1)\n"
  "  endif()\n"
  "endif()\n"
  "if(${CMAKE_PROJECT_NAME}_FIND_QUIETLY)\n"
  "  set(_quiet QUIET)\n"
  "endif()\n\n"
)
foreach(_transient ${${UPPER_PROJECT_NAME}_TRANSIENT_LIBRARIES})
  list(APPEND TRANSIENTS
    "find_package(${_transient} ${${${_transient}_name}_VERSION} EXACT \${_req} \${_quiet})\n"
    "list(APPEND ${UPPER_PROJECT_NAME}_LIBRARIES \${${${_transient}_name}_LIBRARIES})\n"
    "string(TOUPPER ${_transient} _TRANSIENT)\n"
    "if(NOT \${_TRANSIENT}_FOUND)\n"
    "  set(_fail TRUE)\n"
    "endif()\n\n")
endforeach()
string(REGEX REPLACE ";" " " TRANSIENTS ${TRANSIENTS})

# create ProjectConfig.cmake
configure_package_config_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${CMAKE_MODULE_INSTALL_PATH}
  PATH_VARS INCLUDE_INSTALL_DIR LIBRARY_NAMES TRANSIENTS)

# create ProjectConfigVersion.cmake
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
  VERSION ${VERSION} COMPATIBILITY AnyNewerVersion)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
  DESTINATION ${CMAKE_MODULE_INSTALL_PATH})
