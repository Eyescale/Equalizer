# Copyright (c) 2012 Stefan.Eilemann@epfl.ch
# See doc/GitTargets.md for documentation

# Options:
#  GITTARGETS_RELEASE_BRANCH current | even_minor | minor
#      create tags on the current, the next even minor version (e.g. 1.6) or for
#      each minor version

if(GITTARGETS_FOUND)
  return()
endif()
find_package(Git)
if(NOT GIT_EXECUTABLE)
  return()
endif()

if(NOT GITTARGETS_RELEASE_BRANCH)
  set(GITTARGETS_RELEASE_BRANCH "even_minor")
endif()

find_program(GZIP_EXECUTABLE gzip)

# branch
math(EXPR _gittargets_ODD_MINOR "${VERSION_MINOR} % 2")
if(_gittargets_ODD_MINOR AND ${GITTARGETS_RELEASE_BRANCH} STREQUAL even_minor)
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

# remove branch
add_custom_target(cut
  COMMAND ${GIT_EXECUTABLE} branch -d ${BRANCH_VERSION}
  COMMAND ${GIT_EXECUTABLE} push origin --delete ${BRANCH_VERSION}
  COMMENT "Remove branch ${BRANCH_VERSION}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  )

# tag on branch
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/gitbranchandtag.cmake
  "# Branch:
   if(\"${GITTARGETS_RELEASE_BRANCH}\" STREQUAL current)
     set(TAG_BRANCH)
   else()
     execute_process(COMMAND ${GIT_EXECUTABLE} branch ${BRANCH_VERSION}
       RESULT_VARIABLE hadbranch ERROR_VARIABLE error
       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
     if(NOT hadbranch)
       execute_process(COMMAND ${GIT_EXECUTABLE} push origin ${BRANCH_VERSION}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
     endif()
     set(TAG_BRANCH ${BRANCH_VERSION})
   endif()

   # Create or move tag
   execute_process(
     COMMAND ${GIT_EXECUTABLE} tag -f release-${VERSION} ${TAG_BRANCH}
     COMMAND ${GIT_EXECUTABLE} push --tags
     RESULT_VARIABLE notdone WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
   if(notdone)
     message(FATAL_ERROR
        \"Error creating tag release-${VERSION} on branch ${TAG_BRANCH}\")
   endif()")

add_custom_target(tag
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/gitbranchandtag.cmake
  COMMENT "Add tag release-${VERSION}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  )

# remove tag
add_custom_target(erase
  COMMAND ${GIT_EXECUTABLE} tag -d release-${VERSION}
  COMMAND ${GIT_EXECUTABLE} push origin :release-${VERSION}
  COMMENT "Remove tag release-${VERSION}"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  )

# tarball
if(NOT LAST_RELEASE)
  set(LAST_RELEASE ${VERSION})
endif()
set(TARBALL "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}-${LAST_RELEASE}.tar")

add_custom_target(tarball-create
  COMMAND ${GIT_EXECUTABLE} archive --worktree-attributes
    --prefix ${CMAKE_PROJECT_NAME}-${LAST_RELEASE}/ -o ${TARBALL}
    release-${LAST_RELEASE}
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  COMMENT "Creating ${TARBALL}"
  )

if(GZIP_EXECUTABLE)
  add_custom_target(tarball
    COMMAND ${CMAKE_COMMAND} -E remove ${TARBALL}.gz
    COMMAND ${GZIP_EXECUTABLE} ${TARBALL}
    DEPENDS tarball-create
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Compressing ${TARBALL}.gz"
  )
  set(TARBALL_GZ "${TARBALL}.gz")
else()
  add_custom_target(tarball DEPENDS tarball-create)
endif()

set(_gittargets_TARGETS branch cut tag erase tarball tarball-create)
foreach(_gittargets_TARGET ${_gittargets_TARGETS})
  set_target_properties(${_gittargets_TARGET} PROPERTIES EXCLUDE_FROM_ALL ON)
  set_target_properties(${_gittargets_TARGET} PROPERTIES FOLDER "git")
endforeach()
set(GITTARGETS_FOUND 1)
