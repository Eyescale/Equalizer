##
# Copyright (c) 2010 Daniel Pfeifer, All rights reserved.
#
# This file is freely distributable without licensing fees and
# is provided without guarantee or warrantee expressed or implied.
# This file is -not- in the public domain.
##

# create forwarding headers
function(PURPLE_FORWARD_HEADERS PREFIX)
  set(FWD_PREFIX "${CMAKE_BINARY_DIR}/include/${PREFIX}")
  foreach(FILE ${ARGN})
    get_filename_component(ABSOLUTE ${FILE} ABSOLUTE)
    file(RELATIVE_PATH RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ABSOLUTE})
    set(FWDFILE "${FWD_PREFIX}${RELATIVE}")
    if(NOT EXISTS ${FWDFILE})
      get_filename_component(PATH ${RELATIVE} PATH)
      file(RELATIVE_PATH INCLUDE "${FWD_PREFIX}${PATH}" ${ABSOLUTE})
      file(WRITE ${FWDFILE} "#include \"${INCLUDE}\"\n")
    endif(NOT EXISTS ${FWDFILE})
  endforeach(FILE ${ARGN})
endfunction(PURPLE_FORWARD_HEADERS PREFIX)

# create 'include all' header
function(PURPLE_INCLUDE_ALL_HEADER FILE PREFIX)
  set(ALL_FILE ${CMAKE_BINARY_DIR}/include/${FILE})

  string(TOUPPER ${FILE} UPPER)
  string(REGEX REPLACE "[./]" "_" INCLUDE_GUARD ${UPPER})

  file(WRITE ${ALL_FILE}.in
    "/* This file simply includes all \"${PREFIX}\" headers */\n\n"
    "#ifndef ${INCLUDE_GUARD}\n"
    "#define ${INCLUDE_GUARD}\n\n"
    )

  foreach(FILE ${ARGN})
    if(FILE MATCHES "[.]hp*$")
      file(APPEND ${ALL_FILE}.in "#include <${PREFIX}${FILE}>\n")
    endif(FILE MATCHES "[.]hp*$")
  endforeach(FILE ${ARGN})

  file(APPEND ${ALL_FILE}.in "\n#endif /* ${INCLUDE_GUARD} */\n")

  configure_file(${ALL_FILE}.in ${ALL_FILE} COPYONLY)
  string(REGEX MATCH "(.*)[/\\]" DIR ${ALL_FILE})
  install(FILES ${ALL_FILE} DESTINATION ${DIR} COMPONENT dev)
endfunction(PURPLE_INCLUDE_ALL_HEADER FILE PREFIX)
