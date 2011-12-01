
# Copyright (c) 2011 Stefan Eilemann <eile@eyescale.ch>

find_package(Doxygen)
if(NOT DOXYGEN_FOUND)
  return()
endif()

configure_file(DoxygenLayout.xml ${CMAKE_CURRENT_BINARY_DIR}/DoxygenLayout.xml
  @ONLY)
configure_file(doxyfooter.html ${CMAKE_CURRENT_BINARY_DIR}/doxyfooter.html
  @ONLY)
configure_file(Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

add_custom_target(gpusd_doxygen ALL
  ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generating API documentation using doxygen" VERBATIM)

make_directory(${CMAKE_CURRENT_BINARY_DIR}/html) # needed for install:
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/gpu-sd
  COMPONENT gpusddev)
install(FILES AUTHORS  ../LICENSE.txt LGPL.txt GPL.txt
  DESTINATION share/gpu-sd/doc COMPONENT gpusdruntime)
