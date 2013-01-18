
# Copyright (c) 2012-2013 Daniel Nachbaur <daniel.nachbaur@epfl.ch>

# Offers a target named 'module' to create a GNU module
# (http://modules.sourceforge.net/) of your software.
#
# The GNUModules.cmake is supposed to be included after Common.cmake,
# CPackConfig.cmake and all targets to gather required variables from
# them.
#
# Following variables you can change to override defaults:
# - MODULE_ENV: default setup is:
#                 setenv ${UPPER_PROJECT_NAME}_INCLUDE_DIR  $root/include
#                 setenv ${UPPER_PROJECT_NAME}_ROOT         $root
#                 prepend-path PATH            $root/bin
#                 prepend-path LD_LIBRARY_PATH $root/lib
#                 prepend-path PYTHONPATH      $root/${PYTHON_LIBRARY_PREFIX}
#
# - MODULE_SW_BASEDIR: the directory for the module's binaries on a machine.
#                      default /usr/share/Modules
#
# - MODULE_SW_CLASS: the category/subdirectory inside the basedir for this software.
#                    default ${CPACK_PACKAGE_VENDOR}
#
# - MODULE_MODULEFILES: the directory for the modulefiles.
#                       default /usr/share/Modules/modulefiles
#
# - MODULE_WHATIS: the whatis description of the module.
#                  default ${CMAKE_PROJECT_NAME} version ${VERSION}
#
# - MODULE_DEPENDENCIES: list of dependend modules with format:
#                        ${CMAKE_PROJECT_NAME}/${VERSION_MAJOR}.${VERSION_MINOR}

if(MSVC)
  return()
endif()

# Need variables defined by (Common)CPackConfig
if(NOT CPACK_PACKAGE_VENDOR OR NOT VERSION OR NOT CMAKE_SYSTEM_PROCESSOR OR
   NOT LSB_RELEASE OR NOT LSB_DISTRIBUTOR_ID)
  message(FATAL_ERROR "Need CommonCPack before GNUModule")
endif()

if(NOT MODULE_ENV)
  string(TOUPPER ${CMAKE_PROJECT_NAME} UPPER_PROJECT_NAME)
  set(MODULE_ENV
    "setenv ${UPPER_PROJECT_NAME}_INCLUDE_DIR  $root/include\n"
    "setenv ${UPPER_PROJECT_NAME}_ROOT         $root\n\n"
    "prepend-path PATH            $root/bin\n"
    "prepend-path LD_LIBRARY_PATH $root/lib\n")
  if(PYTHON_LIBRARY_PREFIX)
    list(APPEND MODULE_ENV
      "prepend-path PYTHONPATH      $root/${PYTHON_LIBRARY_PREFIX}\n")
  endif()
endif()

if(NOT MODULE_SW_BASEDIR)
  set(MODULE_SW_BASEDIR $ENV{MODULE_SW_BASEDIR})
endif()
if(NOT MODULE_SW_BASEDIR)
  set(MODULE_SW_BASEDIR "/usr/share/Modules")
endif()

if(NOT MODULE_SW_CLASS)
  set(MODULE_SW_CLASS ${CPACK_PACKAGE_VENDOR})
endif()
if(MODULE_SW_CLASS MATCHES "^http://")
  string(REGEX REPLACE "^http://(.*)" "\\1" MODULE_SW_CLASS ${MODULE_SW_CLASS})
endif()

if(NOT MODULE_MODULEFILES)
  set(MODULE_MODULEFILES "/usr/share/Modules/modulefiles")
endif()

if(NOT MODULE_WHATIS)
  set(MODULE_WHATIS "${CMAKE_PROJECT_NAME} version ${VERSION}")
endif()


###############################################################################


# get the used compiler + its version
get_filename_component(MODULE_COMPILER_NAME ${CMAKE_C_COMPILER} NAME CACHE)
include(CompilerVersion)
compiler_dumpversion(MODULE_COMPILER_VERSION)

# setup the module file content
set(MODULE_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(MODULE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR})
if(LSB_DISTRIBUTOR_ID MATCHES "RedHatEnterpriseServer")
  set(MODULE_PLATFORM "rhel${LSB_RELEASE}-${CMAKE_SYSTEM_PROCESSOR}")
elseif(LSB_DISTRIBUTOR_ID MATCHES "Ubuntu")
  set(MODULE_PLATFORM "ubuntu${LSB_RELEASE}-${CMAKE_SYSTEM_PROCESSOR}")
elseif(APPLE)
  set(MODULE_PLATFORM "darwin${CMAKE_SYSTEM_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
else()
  message(WARNING "Unsupported platform for GNUModules, please add support here")
  return()
endif()
set(MODULE_COMPILER "${MODULE_COMPILER_NAME}${MODULE_COMPILER_VERSION}")
set(MODULE_ARCHITECTURE "$platform/$compiler")
set(MODULE_ROOT "$sw_basedir/$sw_class/$package_name/$version/$architecture")
set(MODULE_FILENAME "${MODULE_PACKAGE_NAME}/${MODULE_VERSION}-${MODULE_PLATFORM}-${MODULE_COMPILER}")

# Load dependend modules if any
if(MODULE_DEPENDENCIES)
  foreach(MODULE_DEP ${MODULE_DEPENDENCIES})
    list(INSERT MODULE_ENV 0
      "module load ${MODULE_DEP}-${MODULE_PLATFORM}-${MODULE_COMPILER}\n"
      "prereq      ${MODULE_DEP}-${MODULE_PLATFORM}-${MODULE_COMPILER}\n\n")
  endforeach()
endif()

string(REGEX REPLACE ";" "" MODULE_ENV ${MODULE_ENV})

file(WRITE ${CMAKE_BINARY_DIR}/${MODULE_FILENAME}
  "#%Module1.0\n"
  "######################################################################\n"
  "#\n"
  "# Module:      ${MODULE_FILENAME}\n"
  "#\n"
  "#\n"
  "\n"
  "# Set internal variables\n"
  "set sw_basedir   \"${MODULE_SW_BASEDIR}\"\n"
  "set sw_class     \"${MODULE_SW_CLASS}\"\n"
  "set package_name \"${MODULE_PACKAGE_NAME}\"\n"
  "set version      \"${MODULE_VERSION}\"\n"
  "set platform     \"${MODULE_PLATFORM}\"\n"
  "set compiler     \"${MODULE_COMPILER}\"\n"
  "set architecture \"${MODULE_ARCHITECTURE}\"\n"
  "set root         \"${MODULE_ROOT}\"\n"
  "\n"
  "module-whatis \"${MODULE_WHATIS}\"\n"
  "\n"
  "proc ModulesHelp { } {\n"
  "    global package_name version architecture\n"
  "\n"
  "    puts stderr \"This module prepares your environment to run $package_name $version "
  "for the architecture: $architecture\n"
  "\n"
  "Type 'module list' to list all the loaded modules.\n"
  "Type 'module avail' to list all the availables ones.\"\n"
  "}\n"
  "\n"
  "${MODULE_ENV}\n"
  "\n"
)

get_property(INSTALL_DEPENDS GLOBAL PROPERTY ALL_DEP_TARGETS)
set(MODULE_SRC_INSTALL "${MODULE_SW_BASEDIR}/${MODULE_SW_CLASS}/${MODULE_PACKAGE_NAME}/${MODULE_VERSION}/${MODULE_PLATFORM}/${MODULE_COMPILER}")
add_custom_target(module_install
  ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${MODULE_SRC_INSTALL} -P ${CMAKE_BINARY_DIR}/cmake_install.cmake
  COMMENT "Installing GNU module source at ${MODULE_SRC_INSTALL}" VERBATIM
  DEPENDS ${ALL_DEP_TARGETS})

set(MODULE_FILE_INSTALL ${MODULE_MODULEFILES})
add_custom_target(module
  ${CMAKE_COMMAND} -E copy ${MODULE_FILENAME} ${MODULE_FILE_INSTALL}/${MODULE_FILENAME}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Creating GNU module ${MODULE_FILENAME} at ${MODULE_FILE_INSTALL}" VERBATIM)
add_dependencies(module module_install)
