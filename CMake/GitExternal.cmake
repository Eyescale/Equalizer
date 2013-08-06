# configures an external git repository
# Usage: git_external(<directory> <giturl> <gittag> [RESET <files>])

find_package(Git)
if(NOT GIT_EXECUTABLE)
  return()
endif()

include(CMakeParseArguments)

function(GIT_EXTERNAL DIR REPO TAG)
  cmake_parse_arguments(GIT_EXTERNAL "" "" "RESET" ${ARGN})
  get_filename_component(GIT_EXTERNAL_DIR "${DIR}/.." ABSOLUTE)

  if(NOT EXISTS "${DIR}")
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" clone "${REPO}" "${DIR}"
      RESULT_VARIABLE nok ERROR_VARIABLE error
      WORKING_DIRECTORY "${GIT_EXTERNAL_DIR}")
    if(nok)
      message(FATAL_ERROR "${DIR} git clone failed: ${error}\n")
    endif()
  endif()

  if(IS_DIRECTORY "${DIR}/.git")
    foreach(GIT_EXTERNAL_RESET_FILE ${GIT_EXTERNAL_RESET})
      execute_process(
        COMMAND "${GIT_EXECUTABLE}" reset -q "${GIT_EXTERNAL_RESET_FILE}"
        RESULT_VARIABLE nok ERROR_VARIABLE error
        WORKING_DIRECTORY "${DIR}")
      execute_process(
        COMMAND "${GIT_EXECUTABLE}" checkout -q -- "${GIT_EXTERNAL_RESET_FILE}"
        RESULT_VARIABLE nok ERROR_VARIABLE error
        WORKING_DIRECTORY "${DIR}")
    endforeach()

    execute_process(COMMAND "${GIT_EXECUTABLE}" pull
      RESULT_VARIABLE nok ERROR_VARIABLE error
      WORKING_DIRECTORY "${DIR}")
    if(nok)
      message(WARNING "Update of ${DIR} failed: ${error}\n")
    endif()

    execute_process(
      COMMAND "${GIT_EXECUTABLE}" checkout -q "${TAG}"
      RESULT_VARIABLE nok ERROR_VARIABLE error
      WORKING_DIRECTORY "${DIR}"
      )
    if(nok)
      message(FATAL_ERROR "${DIR} git checkout ${TAG} failed: ${error}\n")
    endif()
  endif()
endfunction()
