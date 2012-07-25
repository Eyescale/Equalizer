# Copyright (c) 2012 Stefan.Eilemann@epfl.ch
# See doc/GitTargets.md for documentation

find_package(Git)
if(NOT GIT_EXECUTABLE)
  return()
endif()

find_program(GZIP_EXECUTABLE gzip)

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

add_custom_target(tarball-create
  COMMAND ${GIT_EXECUTABLE} archive --worktree-attributes
    --prefix ${CMAKE_PROJECT_NAME}-${VERSION}/
    -o ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${VERSION}.tar
    release-${VERSION}
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  COMMENT "Creating ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${VERSION}.tar"
  )

if(GZIP_EXECUTABLE)
  add_custom_target(tarball
    COMMAND ${CMAKE_COMMAND} -E remove "${CMAKE_PROJECT_NAME}-${VERSION}.tar.gz"
    COMMAND ${GZIP_EXECUTABLE} "${CMAKE_PROJECT_NAME}-${VERSION}.tar"
    DEPENDS tarball-create
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT
      "Compressing ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${VERSION}.tar.gz"
  )
else()
  add_custom_target(tarball DEPENDS tarball-create)
endif()

set(_gittargets_TARGETS branch cut tag erase tarball tarball-create)
foreach(_gittargets_TARGET ${_gittargets_TARGETS})
  set_target_properties(${_gittargets_TARGET} PROPERTIES EXCLUDE_FROM_ALL ON)
  set_target_properties(${_gittargets_TARGET} PROPERTIES FOLDER "git")
endforeach()
