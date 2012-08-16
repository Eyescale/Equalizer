# generates MacPorts Portfiles

if(NOT APPLE)
  return()
endif()
if(NOT CPACK_MACPORTS_CATEGORY)
  message("Missing CPACK_MACPORTS_CATEGORY for MacPorts generation")
  return()
endif()
if(NOT CPACK_MACPORTS_DEPENDS)
  message("Missing CPACK_MACPORTS_DEPENDS for MacPorts generation")
  return()
endif()

include(UpdateFile)

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
set(PORTFILE "${CMAKE_BINARY_DIR}/${PORTFILE_DIR}/Portfile")

update_file(${CMAKE_SOURCE_DIR}/CMake/Portfile ${PORTFILE})
install(FILES ${PORTFILE} DESTINATION ${PORTFILE_DIR} COMPONENT lib)
install(CODE
  "execute_process(COMMAND /opt/local/bin/portindex ${CMAKE_INSTALL_PREFIX}/ports)"
  COMPONENT lib)
