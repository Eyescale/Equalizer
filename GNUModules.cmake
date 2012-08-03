
# Copyright (c) 2012 Daniel Nachbaur <daniel.nachbaur@epfl.ch>

# Offers a target named 'module' to create a GNU module
# (http://modules.sourceforge.net/) from your software.
#
# The GNUModules.cmake is supposed to be included after Common.cmake and
# CPackConfig.cmake to gather required variables from them.
# Additionally, the following variables have to set before including
# GNUModules.cmake.

# Need variables defined by (Common)CPackConfig
if(NOT CPACK_PROJECT_NAME OR NOT CPACK_PACKAGE_VENDOR OR NOT VERSION OR
   NOT CMAKE_SYSTEM_PROCESSOR OR NOT LSB_RELEASE OR NOT LSB_DISTRIBUTOR_ID)
  message(FATAL_ERROR "Need CommonCPack before GNUModule")
endif()

# Specify the environment for your software that the module should setup
# Example:
#   set(MODULE_ENV "
#   setenv         FOO_INCLUDE_DIR  $root/include
#   setenv         FOO_ROOT         $root
#   append-path    PATH             $root/bin
#   append-path    LD_LIBRARY_PATH  $root/lib")
if(NOT MODULE_ENV)
  string(TOUPPER ${CMAKE_PROJECT_NAME} UPPER_PROJECT_NAME)
  set(MODULE_ENV "
  setenv         ${UPPER_PROJECT_NAME}_INCLUDE_DIR  $root/include
  setenv         ${UPPER_PROJECT_NAME}_ROOT         $root
  append-path    PATH             $root/bin
  append-path    LD_LIBRARY_PATH  $root/lib")
endif()

# the base directory containing all modules on a machine
if(NOT MODULE_SW_BASEDIR)
  set(MODULE_SW_BASEDIR "/usr/share/Modules")
endif()

# the category/subdirectory inside the basedir for this software
if(NOT MODULE_SW_CLASS)
  set(MODULE_SW_CLASS ${CPACK_PACKAGE_VENDOR})
endif()

# path in MODULE_SW_BASEDIR to modulefiles
if(NOT MODULE_MODULEFILES)
  set(MODULE_MODULEFILES "modulefiles")
endif()

# optional: list of required modules that need to be loaded before this module
if(NOT MODULE_PREREQ)
  set(MODULE_PREREQ "none")
endif()


###############################################################################


# get the used compiler + its version
get_filename_component(MODULE_COMPILER_NAME ${CMAKE_C_COMPILER} NAME CACHE)
include(CompilerVersion)
COMPILER_DUMPVERSION(MODULE_COMPILER_VERSION)

# setup the module file content
set(MODULE_PACKAGE_NAME ${CPACK_PROJECT_NAME})
set(MODULE_VERSION ${VERSION})
if(LSB_DISTRIBUTOR_ID MATCHES "RedHatEnterpriseServer")
  set(MODULE_PLATFORM "rhel${LSB_RELEASE}-${CMAKE_SYSTEM_PROCESSOR}")
elseif(LSB_DISTRIBUTOR_ID MATCHES "Ubuntu")
  set(MODULE_PLATFORM "ubuntu${LSB_RELEASE}-${CMAKE_SYSTEM_PROCESSOR}")
else()
  message(STATUS "Unsupported platform for GNUModules, please add support here")
  return()
endif()
set(MODULE_COMPILER "${MODULE_COMPILER_NAME}${MODULE_COMPILER_VERSION}")
set(MODULE_ARCHITECTURE "$platform/$compiler")
set(MODULE_ROOT "$sw_basedir/$sw_class/$package_name/$version/$architecture")
set(MODULE_FILENAME "${MODULE_PACKAGE_NAME}/${MODULE_VERSION}-${MODULE_PLATFORM}-${MODULE_COMPILER}")

file(WRITE ${CMAKE_BINARY_DIR}/${MODULE_FILENAME}
  "#%Module1.0\n"
  "######################################################################\n"
  "#\n"
  "# Module:      ${MODULE_FILENAME}\n"
  "#\n"
  "# Prereq:\n"
  "#   ${MODULE_PREREQ}\n"
  "#\n"
  "\n"
  "# Set internal variables\n"
  "set             sw_basedir      \"${MODULE_SW_BASEDIR}\"\n"
  "set             sw_class        \"${MODULE_SW_CLASS}\"\n"
  "\n"
  "set             package_name    \"${MODULE_PACKAGE_NAME}\"\n"
  "set             version         \"${MODULE_VERSION}\"\n"
  "set             platform        \"${MODULE_PLATFORM}\"\n"
  "set             compiler        \"${MODULE_COMPILER}\"\n"
  "set             architecture    \"${MODULE_ARCHITECTURE}\"\n"
  "\n"
  "set             root            \"${MODULE_ROOT}\"\n"
  "\n"
  "module-whatis   \"Loads the environment for $package_name\"\n"
  "\n"
  "proc ModulesHelp { }\n"
  "{\n"
  "    global package_name version architecture\n"
  "\n"
  "    puts stderr \"This module prepares your environment to run $package_name $version\n"
  "                 for the architecture: $architecture\n"
  "\n"
  "                 Type 'module list' to list all the loaded modules.\n"
  "                 Type 'module avail' to list all the availables ones.\"\n"
  "}\n"
  "\n"
  "# Update PATH environment:\n"
  "${MODULE_ENV}\n"
  "\n"
)

get_property(INSTALL_DEPENDS GLOBAL PROPERTY ALL_DEP_TARGETS)
set(MODULE_SRC_INSTALL "${MODULE_SW_BASEDIR}/${MODULE_SW_CLASS}/${MODULE_PACKAGE_NAME}/${MODULE_VERSION}/${MODULE_PLATFORM}/${MODULE_COMPILER}")
add_custom_target(module_install
  ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${MODULE_SRC_INSTALL} -P ${CMAKE_BINARY_DIR}/cmake_install.cmake
  COMMENT "Installing GNU module source at ${MODULE_SRC_INSTALL}" VERBATIM
  DEPENDS ${ALL_DEP_TARGETS})

set(MODULE_FILE_INSTALL "${MODULE_SW_BASEDIR}/${MODULE_MODULEFILES}")
add_custom_target(module
  ${CMAKE_COMMAND} -E copy ${MODULE_FILENAME} ${MODULE_FILE_INSTALL}/${MODULE_FILENAME}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Creating GNU module ${MODULE_FILENAME} at ${MODULE_FILE_INSTALL}" VERBATIM)
add_dependencies(module module_install)

