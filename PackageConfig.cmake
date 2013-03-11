
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
# reset before using them
  "set(_output_type)\n"
  "set(_out)\n"
  "set(_req)\n"
  "set(_quiet)\n"
  "set(_fail)\n"
  "\n"
# add transient library finding
  "@TRANSIENTS@"
  "if(NOT _fail)\n"
# setup VERSION, INCLUDE_DIRS and DEB_DEPENDENCIES
  "  set(${UPPER_PROJECT_NAME}_VERSION ${VERSION})\n"
  "  set_and_check(${UPPER_PROJECT_NAME}_INCLUDE_DIRS "@PACKAGE_INCLUDE_INSTALL_DIR@")\n"
  "  set(${UPPER_PROJECT_NAME}_DEB_DEPENDENCIES \"${LOWER_PROJECT_NAME}${VERSION_ABI} (>= ${VERSION_MAJOR}.${VERSION_MINOR})\")\n"
  "\n"
# find components if specified
  "  if(${CMAKE_PROJECT_NAME}_FIND_COMPONENTS)\n"
  "    find_library(\${UPPER_PROJECT_NAME}_LIBRARY ${CMAKE_PROJECT_NAME} NO_DEFAULT_PATH\n"
  "                 PATHS \${PACKAGE_PREFIX_DIR} PATH_SUFFIXES lib ${PYTHON_LIBRARY_PREFIX})\n"
  "    list(APPEND ${UPPER_PROJECT_NAME}_LIBRARIES \${${UPPER_PROJECT_NAME}_LIBRARY})\n"
  "    foreach(_component \${${CMAKE_PROJECT_NAME}_FIND_COMPONENTS})\n"
  "      find_library(\${_component}_libraryname ${CMAKE_PROJECT_NAME}_\${_component} NO_DEFAULT_PATH\n"
  "        PATHS \${PACKAGE_PREFIX_DIR} PATH_SUFFIXES lib ${PYTHON_LIBRARY_PREFIX})\n"
  "\n"
  "      if(\${_component}_libraryname MATCHES "\${_component}_libraryname-NOTFOUND")\n"
  "        if(${CMAKE_PROJECT_NAME}_FIND_REQUIRED_\${_component})\n"
  "          set(_fail \"Component library \${_component} not found\")\n"
  "          message(FATAL_ERROR \"   ${CMAKE_PROJECT_NAME}_\${_component} \"\n"
  "            \"not found in \${PACKAGE_PREFIX_DIR}/lib\")\n"
  "        elseif(NOT _quiet)\n"
  "          message(STATUS \"   ${CMAKE_PROJECT_NAME}_\${_component} \"\n"
  "            \"not found in \${PACKAGE_PREFIX_DIR}/lib\")\n"
  "        endif()\n"
  "      else()\n"
  "        string(TOUPPER \${_component} _COMPONENT)\n"
  "        set(${UPPER_PROJECT_NAME}_\${_COMPONENT}_FOUND TRUE)\n"
  "        set(${UPPER_PROJECT_NAME}_\${_COMPONENT}_LIBRARY \${\${_component}_libraryname})\n"
  "        list(APPEND ${UPPER_PROJECT_NAME}_LIBRARIES \${\${_component}_libraryname})\n"
  "        list(APPEND ${UPPER_PROJECT_NAME}_COMPONENTS \${_component})\n"
  "      endif()\n"
  "    endforeach()\n"
  "  else()\n"
# if no component was specified, find all produced libraries
  "    set(${UPPER_PROJECT_NAME}_LIBRARY_NAMES "@LIBRARY_NAMES@")\n"
  "    foreach(_libraryname \${${UPPER_PROJECT_NAME}_LIBRARY_NAMES})\n"
  "      string(TOUPPER \${_libraryname} _LIBRARYNAME)\n"
  "      find_library(\${_LIBRARYNAME}_LIBRARY \${_libraryname} NO_DEFAULT_PATH\n"
  "                   PATHS \${PACKAGE_PREFIX_DIR} PATH_SUFFIXES lib ${PYTHON_LIBRARY_PREFIX})\n"
  "      if(\${_LIBRARYNAME}_LIBRARY MATCHES "\${_LIBRARYNAME}_LIBRARY-NOTFOUND")\n"
  "        set(_fail \"\${_libraryname} not found\")\n"
  "        if(_out)\n"
  "          message(\${_output_type} \"   Missing the ${CMAKE_PROJECT_NAME} \"\n"
  "            \"library in \${PACKAGE_PREFIX_DIR}/lib.\")\n"
  "        endif()\n"
  "      else()\n"
  "        list(APPEND ${UPPER_PROJECT_NAME}_LIBRARIES \${\${_LIBRARYNAME}_LIBRARY})\n"
  "        string(REPLACE \"${CMAKE_PROJECT_NAME}_\" \"\" _component \${_libraryname})\n"
  "        string(TOUPPER \${_component} _COMPONENT)\n"
  "        set(${UPPER_PROJECT_NAME}_\${_COMPONENT}_FOUND TRUE)\n"
  "        list(APPEND ${UPPER_PROJECT_NAME}_COMPONENTS \${_component})\n"
  "      endif()\n"
  "    endforeach()\n"
  "  endif()\n"
  "\n"
# include options.cmake if existing
  "  if(EXISTS \${PACKAGE_PREFIX_DIR}/${CMAKE_MODULE_INSTALL_PATH}/options.cmake)\n"
  "    include(\${PACKAGE_PREFIX_DIR}/${CMAKE_MODULE_INSTALL_PATH}/options.cmake)\n"
  "  endif()\n"
  "endif()\n"
  "\n"
# finally report about found or not found
  "if(_fail)\n"
  "  set(${UPPER_PROJECT_NAME}_FOUND)\n"
  "  set(${UPPER_PROJECT_NAME}_VERSION)\n"
  "  set(${UPPER_PROJECT_NAME}_INCLUDE_DIRS)\n"
  "  set(${UPPER_PROJECT_NAME}_LIBRARIES)\n"
  "  set(${UPPER_PROJECT_NAME}_DEB_DEPENDENCIES)\n"
  "  set(${UPPER_PROJECT_NAME}_LIBRARY)\n"
  "  set(${UPPER_PROJECT_NAME}_COMPONENTS)\n"
  "  if(_out)\n"
  "    message(STATUS \"Could not find ${CMAKE_PROJECT_NAME}: \${_fail}\")\n"
  "  endif()\n"
  "else()\n"
  "  set(${UPPER_PROJECT_NAME}_FOUND TRUE)\n"
  "  set(${UPPER_PROJECT_NAME}_MODULE_FILENAME ${MODULE_FILENAME})\n"
  "  if(_out)\n"
  "    message(STATUS \"Found ${CMAKE_PROJECT_NAME} ${VERSION} \${${UPPER_PROJECT_NAME}_COMPONENTS} in \"\n"
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
  if(${_transient}_FOUND)
    set(${_transient}_name ${_transient})
  endif()
  string(TOUPPER ${_transient} _TRANSIENT)
  if(${_TRANSIENT}_FOUND)
    set(${_transient}_name ${_TRANSIENT})
  endif()
  if(NOT ${_transient}_name)
    message(FATAL_ERROR "Transient library ${_transient} was not properly resolved")
  endif()
  if(${${_transient}_name}_VERSION)
    set(${${_transient}_name}_findmode EXACT)
  else()
    set(${${_transient}_name}_findmode REQUIRED)
  endif()
  list(APPEND TRANSIENTS
    "find_package(${_transient} ${${${_transient}_name}_VERSION} ${${${_transient}_name}_findmode} \${_req} \${_quiet})\n"
    "if(${${_transient}_name}_FOUND)\n"
    "  list(APPEND ${UPPER_PROJECT_NAME}_LIBRARIES \${${${_transient}_name}_LIBRARIES})\n"
    "else()\n"
    "  set(_fail TRUE)\n"
    "endif()\n\n")
endforeach()
string(REGEX REPLACE ";" " " TRANSIENTS ${TRANSIENTS})

# create ProjectConfig.cmake
if(LIBRARY_NAMES)
  set(HAS_LIBRARY_NAMES LIBRARY_NAMES)
endif()

configure_package_config_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${CMAKE_MODULE_INSTALL_PATH}
  PATH_VARS INCLUDE_INSTALL_DIR ${HAS_LIBRARY_NAMES} TRANSIENTS
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

# create ProjectConfigVersion.cmake
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
  VERSION ${VERSION} COMPATIBILITY AnyNewerVersion)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
  DESTINATION ${CMAKE_MODULE_INSTALL_PATH} COMPONENT dev)
