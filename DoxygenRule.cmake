# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>

find_package(Doxygen)
if(NOT DOXYGEN_FOUND)
  return()
endif()

if(NOT DOXYGEN_CONFIG_FILE)
  # Assuming there exists a Doxyfile and that needs configuring
  configure_file(doc/Doxyfile ${CMAKE_BINARY_DIR}/doc/Doxyfile @ONLY)
  set(DOXYGEN_CONFIG_FILE ${CMAKE_BINARY_DIR}/doc/Doxyfile)
endif()

get_property(INSTALL_DEPENDS GLOBAL PROPERTY ALL_DEP_TARGETS)
add_custom_target(doxygen_install
  ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake_install.cmake
  DEPENDS ${ALL_DEP_TARGETS})

add_custom_target(doxygen
  ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG_FILE}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
  COMMENT "Generating API documentation using doxygen" VERBATIM)
add_dependencies(doxygen doxygen_install)

make_directory(${CMAKE_BINARY_DIR}/doc/man/man3)
install(DIRECTORY ${CMAKE_BINARY_DIR}/doc/man/man3 DESTINATION man
  COMPONENT man PATTERN "*_docs_*" EXCLUDE)

make_directory(${CMAKE_BINARY_DIR}/doc/html)
install(DIRECTORY ${CMAKE_BINARY_DIR}/doc/html
  DESTINATION share/${CMAKE_PROJECT_NAME}/doc/API COMPONENT doc)

if(NOT GIT_DOCUMENTATION_REPO)
  include(GithubOrganization)
  set(GIT_DOCUMENTATION_REPO ${GIT_ORIGIN_org})
endif()
if(GIT_DOCUMENTATION_REPO)
  set(GIT_DOCUMENTATION_DIR
    ${CMAKE_SOURCE_DIR}/../${GIT_DOCUMENTATION_REPO}/${PROJECT_NAME}-${VERSION_MAJOR}.${VERSION_MINOR})
  add_custom_target(doxycopy
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${GIT_DOCUMENTATION_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/doc/html ${GIT_DOCUMENTATION_DIR}
    COMMENT "Copying API documentation to ${GIT_DOCUMENTATION_DIR}"
    VERBATIM)
  add_dependencies(doxycopy doxygen)
endif()

add_custom_target(doxygit
  COMMAND ${CMAKE_COMMAND} -DCMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}" -DCMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}" -DCMAKE_PROJECT_NAME="${GIT_DOCUMENTATION_REPO}" -P ${CMAKE_CURRENT_LIST_DIR}/Doxygit.cmake
  COMMENT "Updating ${GIT_DOCUMENTATION_REPO}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../${GIT_DOCUMENTATION_REPO}"
  )
add_dependencies(doxygit doxycopy)
