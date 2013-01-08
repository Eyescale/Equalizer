
# Copyright (c) 2013 Daniel Nachbaur <daniel.nachbaur@epfl.ch>

# This file creates package information for find_package by generating
# ${CMAKE_PROJECT_NAME}Config.cmake and ${CMAKE_PROJECT_NAME}ConfigVersion.cmake
# files. Those files are used in the config-mode of find_package which
# supersedes the Find${CMAKE_PROJECT_NAME}.cmake file.
#
# TODO
# - LIBRARIES
# - transient packages
# - option.cmake

include(CMakePackageConfigHelpers)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake.in
  "set(${CMAKE_PROJECT_NAME}_VERSION ${VERSION})\n"
  "\n"
  "@PACKAGE_INIT@\n"
  "\n"
  "set_and_check(${UPPER_PROJECT_NAME}_INCLUDE_DIR "@PACKAGE_INCLUDE_INSTALL_DIR@")\n"
  "set_and_check(${UPPER_PROJECT_NAME}_LIBRARIES "@PACKAGE_LIBRARIES@")\n"
  "set(${UPPER_PROJECT_NAME}_DEB_DEPENDENCIES \"${LOWER_PROJECT_NAME}${VERSION_ABI}-lib (>= ${VERSION_MAJOR}.${VERSION_MINOR})\")\n"
  "\n"
  "check_required_components(${CMAKE_PROJECT_NAME})\n"
)

set(INCLUDE_INSTALL_DIR include)
set(LIBRARIES dummy)
configure_package_config_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${CMAKE_MODULE_INSTALL_PATH}
  PATH_VARS INCLUDE_INSTALL_DIR LIBRARIES)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
  VERSION ${VERSION} COMPATIBILITY AnyNewerVersion)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake
  DESTINATION ${CMAKE_MODULE_INSTALL_PATH})
