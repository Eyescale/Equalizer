# Copyright (c) 2012 Stefan.Eilemann@epfl.ch

# Adds the following targets for git-based repositories:
#
# branch: Create MAJOR.MINOR branch and push it
# cut: Delete MAJOR.MINOR locally and remote
# tag: branch, switch to branch and create or move release-VERSION tag to HEAD
# erase: Remove release-VERSION tag
#
# NB: Branch minor versions are rounded up to the next even version!

find_package(Git REQUIRED)

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
        \"Error creating tag release-${VERSION} on ${BRANCH_VERSION}\")
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

set(_gittargets_TARGETS branch cut tag erase)
foreach(_gittargets_TARGET ${_gittargets_TARGETS})
  set_target_properties(${_gittargets_TARGET} PROPERTIES EXCLUDE_FROM_ALL ON)
  set_target_properties(${_gittargets_TARGET} PROPERTIES FOLDER "git")
endforeach()
