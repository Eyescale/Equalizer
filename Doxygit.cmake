# used by DoxygenRule.cmake, don't use directly

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/oss)
find_package(Git)

if(NOT GIT_EXECUTABLE)
  return()
endif()
include(UpdateFile)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/index.html"
"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd\">\n"
"<html>\n"
"  <head>\n"
"    <title>${CMAKE_PROJECT_NAME} API Documentation</title>\n"
"    <link rel=\"stylesheet\" href=\"CMake/github.css\" type=\"text/css\">"
"  </head>\n"
"  <body>\n"
"  <h1>API Documentation</h1>"
"  <table>\n"
)

file(GLOB ENTRIES RELATIVE ${CMAKE_SOURCE_DIR} *-*)
set(VERSIONS)
set(PROJECTS)

# Create project and version lists
foreach(ENTRY ${ENTRIES})
  string(REGEX REPLACE "^(.+)-.+$" "\\1" PROJECT ${ENTRY})
  string(REGEX REPLACE "^.+-(.+)$" "\\1" VERSION ${ENTRY})

  list(FIND VERSIONS "${VERSION}" VERSION_FOUND)
  if(VERSION_FOUND EQUAL -1)
    list(APPEND VERSIONS "${VERSION}")
  endif()

  list(FIND PROJECTS "${PROJECT}" PROJECT_FOUND)
  if(PROJECT_FOUND EQUAL -1)
    list(APPEND PROJECTS "${PROJECT}")
  endif()
endforeach()
list(SORT VERSIONS)
list(REVERSE VERSIONS)
list(SORT PROJECTS)

# generate version table
list(LENGTH VERSIONS NUM_VERSIONS)
set(GIT_DOCUMENTATION_INSTALL)

file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/index.html"
  "    <tr><th>Project</th><th colspan=\"${NUM_VERSIONS}\">Versions</th>")
if(DOXYGIT_PROJECT_EXTRA)
  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/index.html" "<th>Status</th>")
endif()

foreach(PROJECT ${PROJECTS})
  if(DOXYGIT_PROJECT_EXTRA)
    string(REPLACE "[PROJECT]" "${PROJECT}" DOXYGIT_PROJECT_EXTRA_
      ${DOXYGIT_PROJECT_EXTRA})
  endif()
  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/index.html"
    "</tr>\n"
    "    <tr><th><a href=\"https://github.com/${CMAKE_PROJECT_NAME}/${PROJECT}\">${PROJECT}</a></th>")
  foreach(VERSION ${VERSIONS})
    set(ENTRY "${PROJECT}-${VERSION}")
    list(FIND ENTRIES "${ENTRY}" HAS_ENTRY)
    if(HAS_ENTRY EQUAL -1)
      file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/index.html" "<th></th>")
    else()
      file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/index.html"
        "<td><a href=\"${ENTRY}/index.html\">${VERSION}</a></td>")
      list(APPEND GIT_DOCUMENTATION_INSTALL ${ENTRY})
    endif()
  endforeach()
  if(DOXYGIT_PROJECT_EXTRA_)
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/index.html"
      "<th>${DOXYGIT_PROJECT_EXTRA_}</th>")
  endif()
endforeach()

execute_process(COMMAND "${GIT_EXECUTABLE}" add images ${ENTRIES}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/index.html"
"</tr>\n"
"  </table>\n"
"  <h1>Project Dependencies</h1>"
"  <a href=\"images/all.png\"><img src=\"images/all.png\" width=100%></a>"
"  </body>\n"
"</html>\n"
)

update_file("${CMAKE_CURRENT_BINARY_DIR}/index.html"
  "${CMAKE_SOURCE_DIR}/index.html")

foreach(FOLDER ${GIT_DOCUMENTATION_INSTALL})
  install(DIRECTORY ${FOLDER} DESTINATION share/${CMAKE_PROJECT_NAME}
    CONFIGURATIONS Release)
endforeach()
install(FILES index.html DESTINATION share/${CMAKE_PROJECT_NAME}
  CONFIGURATIONS Release)
