# generates MacPorts Portfiles

if(NOT APPLE)
  return()
endif()
if(NOT CPACK_MACPORTS_CATEGORY)
  message("Missing CPACK_MACPORTS_CATEGORY for MacPorts generation")
  return()
endif()

include(UpdateFile)
include(GithubOrganization)

if(NOT CPACK_MACPORTS_VERSION)
  set(CPACK_MACPORTS_VERSION ${LAST_RELEASE})
endif()
if(NOT CPACK_MACPORTS_VERSION)
  set(CPACK_MACPORTS_VERSION ${VERSION})
endif()

# format dependencies list into port:name string
foreach(CPACK_MACPORTS_DEPEND ${CPACK_MACPORTS_DEPENDS})
  if(${CPACK_MACPORTS_DEPEND} MATCHES "port:")
    set(CPACK_MACPORTS_TEMP "${CPACK_MACPORTS_TEMP} ${CPACK_MACPORTS_DEPEND}") 
  else()
    set(CPACK_MACPORTS_TEMP
      "${CPACK_MACPORTS_TEMP} port:${CPACK_MACPORTS_DEPEND}") 
  endif()
endforeach()
set(CPACK_MACPORTS_DEPENDS "${CPACK_MACPORTS_TEMP}")

# Create and install Portfile
set(PORTFILE_DIR "ports/${CPACK_MACPORTS_CATEGORY}/${CMAKE_PROJECT_NAME}")
set(PORTFILE_GH_DIR "${CMAKE_SOURCE_DIR}/../${GIT_ORIGIN_org}Ports")
set(PORTFILE "${CMAKE_BINARY_DIR}/${PORTFILE_DIR}/Portfile")
set(PORTFILE_GH "${PORTFILE_GH_DIR}/${PORTFILE_DIR}/Portfile")

update_file(${CMAKE_SOURCE_DIR}/CMake/Portfile ${PORTFILE})
install(FILES ${PORTFILE} DESTINATION ${PORTFILE_DIR} COMPONENT lib)
install(CODE
  "execute_process(COMMAND /opt/local/bin/portindex ${CMAKE_INSTALL_PREFIX}/ports)"
  COMPONENT lib)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/MacPortfile.cmake
    "list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)\n"
    "include(UpdateFile)\n"
    "update_file(${PORTFILE} ${PORTFILE_GH})\n"
    "execute_process(COMMAND /opt/local/bin/portindex ${PORTFILE_GH_DIR}/ports)"
  )

add_custom_target(Portfile
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/MacPortfile.cmake
  COMMENT "Updating ${GIT_ORIGIN_ORG} Ports")
