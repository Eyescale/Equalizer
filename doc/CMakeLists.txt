
# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

if(NOT DOXYGEN_FOUND)
  return()
endif()

if(EQUALIZER_RELEASE)
  set(EQ_DOXYGEN_VERSION ${VERSION})
  set(CO_DOXYGEN_VERSION ${CO_VERSION})
else()
  set(EQ_DOXYGEN_VERSION ${VERSION}-git)
  set(CO_DOXYGEN_VERSION ${CO_VERSION}-git)
endif()

configure_file(DoxygenLayout.xml ${CMAKE_CURRENT_BINARY_DIR}/DoxygenLayout.xml
  @ONLY)
configure_file(doxyfooter.html ${CMAKE_CURRENT_BINARY_DIR}/doxyfooter.html
  @ONLY)
configure_file(Doxyfile.int ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.int @ONLY)

get_property(INSTALL_DEPENDS GLOBAL PROPERTY ALL_TARGETS)
add_custom_target(doxygen_install
  ${CMAKE_COMMAND} -D CMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
                   -P ${CMAKE_BINARY_DIR}/cmake_install.cmake
  DEPENDS ${INSTALL_DEPENDS})

# Note: packages are chained by depends to enforce serial execution.
#       Doxygen is not multiprocess-safe when running in the same directory.
add_custom_target(doxygen_int
  ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.int
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generating internal API documentation using doxygen" VERBATIM)

set(PUBLIC_DOCS Collage Sequel Equalizer)
set(PREVIOUS doxygen_int doxygen_install)
foreach(DOC ${PUBLIC_DOCS})
  configure_file(Doxyfile.${DOC} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.${DOC}
    @ONLY)
  add_custom_target(doxygen_${DOC}
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.${DOC}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating ${DOC} API documentation using doxygen" VERBATIM)
  add_dependencies(doxygen_${DOC} ${PREVIOUS})

  if(DOC STREQUAL Collage)
    set(PROJECT_VERSION ${CO_VERSION})
  else()
    set(PROJECT_VERSION ${VERSION})
  endif()

  add_custom_target(github_${DOC}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/../eyescale/${DOC}-${PROJECT_VERSION}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/doc/html/${DOC} ${CMAKE_SOURCE_DIR}/../eyescale/${DOC}-${PROJECT_VERSION}
    COMMENT "Copying API documentation to eyescale.github.com/${DOC}-${PROJECT_VERSION}"
    VERBATIM)
  add_dependencies(github_${DOC} doxygen_${DOC})

  make_directory(${CMAKE_CURRENT_BINARY_DIR}/html/${DOC})
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/${DOC}
    DESTINATION share/Equalizer COMPONENT doc)
  set(PREVIOUS doxygen_${DOC})
endforeach()

# set_target_properties(tests PROPERTIES FOLDER "Tests")
add_custom_target(doxygen DEPENDS doxygen_Equalizer)
add_custom_target(github DEPENDS github_Collage github_Sequel github_Equalizer)

make_directory(${CMAKE_CURRENT_BINARY_DIR}/man/man3)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/man/man3 DESTINATION man
  COMPONENT man PATTERN "*_docs_*" EXCLUDE)

make_directory(${CMAKE_SOURCE_DIR}/../equalizergraphics/build/documents/Developer/API/internal)
install(DIRECTORY . DESTINATION share/Equalizer/doc COMPONENT doc)
install(FILES ../RELNOTES.txt ../AUTHORS ../CHANGES.txt ../ACKNOWLEDGEMENTS
        ../LGPL ../LICENSE.txt DESTINATION share/Equalizer/doc COMPONENT doc)
