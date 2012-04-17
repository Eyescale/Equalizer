# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>

if(NOT DOXYGEN_FOUND)
  return()
endif()

configure_file(doc/DoxygenLayout.xml ${CMAKE_BINARY_DIR}/doc/DoxygenLayout.xml
  @ONLY)
configure_file(doc/Doxyfile ${CMAKE_BINARY_DIR}/doc/Doxyfile @ONLY)

get_property(INSTALL_DEPENDS GLOBAL PROPERTY DOXYGEN_DEP_TARGETS)
add_custom_target(doxygen_install
  ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake_install.cmake
  DEPENDS ${INSTALL_DEPENDS})

add_custom_target(doxygen
  ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/doc/Doxyfile
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
  COMMENT "Generating API documentation using doxygen" VERBATIM)
add_dependencies(doxygen doxygen_install)

make_directory(${CMAKE_BINARY_DIR}/doc/man/man3)
install(DIRECTORY ${CMAKE_BINARY_DIR}/doc/man/man3 DESTINATION man
  COMPONENT man PATTERN "*_docs_*" EXCLUDE)

make_directory(${CMAKE_BINARY_DIR}/doc/html)
install(DIRECTORY ${CMAKE_BINARY_DIR}/doc/html
  DESTINATION share/${CMAKE_PROJECT_NAME}/doc/API COMPONENT doc)
