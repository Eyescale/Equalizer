##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
##

macro(PURPLE_EXPAND_LIBRARIES VAR)
  set(${VAR})
  set(VARIANT)
  foreach(LIBRARY ${ARGN})
    if(VARIANT STREQUAL "shared")
      list(APPEND ${VAR} lib_${LIBRARY}_shared)
    elseif(VARIANT STREQUAL "static")
      list(APPEND ${VAR} lib_${LIBRARY}_static)
    elseif(NOT LIBRARY STREQUAL "shared" AND NOT LIBRARY STREQUAL "static")
      list(APPEND ${VAR} ${LIBRARY})
    endif()
    set(VARIANT ${LIBRARY})
  endforeach(LIBRARY ${ARGN})
endmacro(PURPLE_EXPAND_LIBRARIES VAR)
