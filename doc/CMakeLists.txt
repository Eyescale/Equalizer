##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
##

find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
  message(STATUS "Install Doxygen to build documentation")
  return()
endif(NOT DOXYGEN_FOUND)

include(PurpleDoxygen)

if(DOXYGEN_DOT_FOUND)
  set(DOT_OPTIONS
    HAVE_DOT=YES
    DOT_PATH=${DOXYGEN_DOT_PATH}
    )
endif(DOXYGEN_DOT_FOUND)

purple_doxygen(doc_internal
  DOXYFILE
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.int
  INPUT
    ${CMAKE_BINARY_DIR}/include/vmmlib
    ${CMAKE_SOURCE_DIR}/libs
    ${CMAKE_SOURCE_DIR}/admin
    ${CMAKE_SOURCE_DIR}/examples
    ${CMAKE_SOURCE_DIR}/server
  PARAMETERS
    ${DOT_OPTIONS}
    EXAMPLE_PATH=${CMAKE_SOURCE_DIR}/lib
    HTML_FOOTER=${CMAKE_CURRENT_SOURCE_DIR}/doxyfooter.html
    LAYOUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/DoxygenLayout.xml
    OUTPUT_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/internal_html
    CREATE_SUBDIRS=YES
  )

#purple_doxygen(doc_external
#  )
