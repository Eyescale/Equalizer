##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
##

# create forwarding headers
function(PURPLE_FORWARD_HEADERS DIR)
  set(FWD_DIR ${CMAKE_BINARY_DIR}/include/${DIR})
  foreach(FILE ${ARGN})
    get_filename_component(ABSOLUTE ${FILE} ABSOLUTE)
    file(RELATIVE_PATH RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${ABSOLUTE})
    set(FWDFILE ${FWD_DIR}/${RELATIVE})
    if(NOT EXISTS ${FWDFILE})
      get_filename_component(PATH ${RELATIVE} PATH)
      file(RELATIVE_PATH INCLUDE ${FWD_DIR}/${PATH} ${ABSOLUTE})
      file(WRITE ${FWDFILE} "#include \"${INCLUDE}\"\n")
    endif(NOT EXISTS ${FWDFILE})
  endforeach(FILE ${ARGN})
endfunction(PURPLE_FORWARD_HEADERS DIR)
