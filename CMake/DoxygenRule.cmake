# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>

find_package(Doxygen)
if(NOT DOXYGEN_FOUND)
  return()
endif()

find_package(Git)
include(GithubOrganization)

configure_file(doc/DoxygenLayout.xml ${CMAKE_BINARY_DIR}/doc/DoxygenLayout.xml
  @ONLY)
configure_file(doc/Doxyfile ${CMAKE_BINARY_DIR}/doc/Doxyfile @ONLY)

get_property(INSTALL_DEPENDS GLOBAL PROPERTY ALL_DEP_TARGETS)
add_custom_target(doxygen_install
  ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake_install.cmake
  DEPENDS ${ALL_DEP_TARGETS})

add_custom_target(doxygen
  ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/Doxyfile
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
  COMMENT "Generating API documentation using doxygen" VERBATIM)
add_dependencies(doxygen doxygen_install)

add_custom_target(github
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/../${GIT_ORIGIN_org}/${PROJECT_NAME}-${VERSION}
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/doc/html ${CMAKE_SOURCE_DIR}/../${GIT_ORIGIN_org}/${PROJECT_NAME}-${VERSION}
  COMMENT "Copying API documentation to ${GIT_ORIGIN_org}.github.com/${PROJECT_NAME}-${VERSION}"
  VERBATIM)
add_dependencies(github doxygen)

make_directory(${CMAKE_BINARY_DIR}/doc/man/man3)
install(DIRECTORY ${CMAKE_BINARY_DIR}/doc/man/man3 DESTINATION man
  COMPONENT man PATTERN "*_docs_*" EXCLUDE)

make_directory(${CMAKE_BINARY_DIR}/doc/html)
install(DIRECTORY ${CMAKE_BINARY_DIR}/doc/html
  DESTINATION share/${CMAKE_PROJECT_NAME}/doc/API COMPONENT doc)
