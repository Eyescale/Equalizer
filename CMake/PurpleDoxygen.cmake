##
# Copyright (c) 2010 Daniel Pfeifer, All rights reserved.
#
# This file is freely distributable without licensing fees and
# is provided without guarantee or warrantee expressed or implied.
# This file is -not- in the public domain.
##

include(ParseArguments)

macro(PURPLE_DOXYGEN NAME)
  parse_arguments(THIS "DOXYFILE;INPUT;PARAMETERS" "" ${ARGN})

  set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.doxyfile)
  
  if(THIS_DOXYFILE)
    configure_file(${THIS_DOXYFILE} ${DOXYFILE} COPYONLY)
  else()
    execute_process(COMMAND ${DOXYGEN_EXECUTABLE} -s -g ${DOXYFILE})
  endif()

  foreach(PARAM ${THIS_PARAMETERS})
    file(APPEND ${DOXYFILE} "${PARAM}\n")
  endforeach(PARAM)

  set(INPUT)
  foreach(FILE ${THIS_INPUT})
    set(INPUT "${INPUT} \\\n \"${FILE}\"")
  endforeach(FILE ${THIS_DEFAULT_ARGS})
  file(APPEND ${DOXYFILE} "INPUT = ${INPUT}\n")

# set(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.ok)
# add_custom_command(OUTPUT ${OUTPUT}
#   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
#   COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}
#   DEPENDS ${THIS_INPUT})
# add_custom_target(${NAME} DEPENDS ${OUTPUT})

  add_custom_target(${NAME} ${DOXYGEN_EXECUTABLE} ${DOXYFILE})
endmacro(PURPLE_DOXYGEN)
