# Copyright (c) 2012 Stefan.Eilemann@epfl.ch
# See doc/GitTargets.md for documentation

find_package(Git)
if(NOT GIT_EXECUTABLE)
  return()
endif()

math(EXPR _gittargets_ODD_MINOR "${VERSION_MINOR} % 2")
if(_gittargets_ODD_MINOR)
  math(EXPR BRANCH_VERSION "${VERSION_MINOR} + 1")
  set(BRANCH_VERSION ${VERSION_MAJOR}.${BRANCH_VERSION})
else()
  set(BRANCH_VERSION ${VERSION_MAJOR}.${VERSION_MINOR})
endif()

add_custom_target(branch
  COMMAND ${GIT_EXECUTABLE} branch ${BRANCH_VERSION}
  COMMAND ${GIT_EXECUTABLE} push origin ${BRANCH_VERSION}
  COMMENT "Add branch ${BRANCH_VERSION}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  )

add_custom_target(cut
  COMMAND ${GIT_EXECUTABLE} branch -d ${BRANCH_VERSION}
  COMMAND ${GIT_EXECUTABLE} push origin --delete ${BRANCH_VERSION}
  COMMENT "Remove branch ${BRANCH_VERSION}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  )

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/gitbranchandtag.cmake
  "# Create branch:
   execute_process(COMMAND ${GIT_EXECUTABLE} branch ${BRANCH_VERSION}
     RESULT_VARIABLE hadbranch ERROR_VARIABLE error
     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
   if(NOT hadbranch)
     execute_process(COMMAND ${GIT_EXECUTABLE} push origin ${BRANCH_VERSION}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
   endif()
   # Create or move tag
   execute_process(
     COMMAND ${GIT_EXECUTABLE} tag -f release-${VERSION} ${BRANCH_VERSION}
     COMMAND ${GIT_EXECUTABLE} push --tags
     RESULT_VARIABLE notdone WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
   if(notdone)
     message(FATAL_ERROR
        \"Error creating tag release-${VERSION} on branch ${BRANCH_VERSION}\")
   endif()")

add_custom_target(tag
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/gitbranchandtag.cmake
  COMMENT "Add tag release-${VERSION}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  )

add_custom_target(erase
  COMMAND ${GIT_EXECUTABLE} tag -d release-${VERSION}
  COMMAND ${GIT_EXECUTABLE} push origin :release-${VERSION}
  COMMENT "Remove tag release-${VERSION}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  )

add_custom_target(tarball-clone
  COMMAND ${CMAKE_COMMAND} -E remove_directory
                              "${CMAKE_PROJECT_NAME}-${VERSION}"
  COMMAND ${GIT_EXECUTABLE} clone "${CMAKE_SOURCE_DIR}"
                                  "${CMAKE_PROJECT_NAME}-${VERSION}"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  COMMENT "Cloning source"
  )

add_custom_target(tarball-prepare
  COMMAND ${GIT_EXECUTABLE} checkout -q release-${VERSION}
  COMMAND ${CMAKE_COMMAND} -E remove_directory .git
  DEPENDS tarball-clone
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${VERSION}"
  COMMENT "Preparing ${CMAKE_PROJECT_NAME}-${VERSION}"
  )

add_custom_target(tarball
  COMMAND ${CMAKE_COMMAND} -E tar czf "${CMAKE_PROJECT_NAME}-${VERSION}.tar.gz"
                                      "${CMAKE_PROJECT_NAME}-${VERSION}"
  DEPENDS tarball-prepare
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  COMMENT "Creating ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${VERSION}.tar.gz"
  )
  
set(_gittargets_TARGETS branch cut tag erase tarball tarball-clone
  tarball-prepare)
foreach(_gittargets_TARGET ${_gittargets_TARGETS})
  set_target_properties(${_gittargets_TARGET} PROPERTIES EXCLUDE_FROM_ALL ON)
  set_target_properties(${_gittargets_TARGET} PROPERTIES FOLDER "git")
endforeach()
