##
# Copyright (c) 2010 Daniel Pfeifer, All rights reserved.
#
# This file is freely distributable without licensing fees and
# is provided without guarantee or warrantee expressed or implied.
# This file is -not- in the public domain.
##

include(ParseArguments)

macro(PURPLE_EXPAND_LIBRARIES VAR)
  set(${VAR})
  set(VARIANT)
  parse_arguments(THIS "EXCLUDE" "" ${ARGN})
  foreach(LIBRARY ${THIS_DEFAULT_ARGS})
    list(FIND THIS_EXCLUDE ${LIBRARY} EXCLUDED)
	if(NOT EXCLUDED EQUAL -1)
	  # skip
    elseif(VARIANT STREQUAL "shared")
      list(APPEND ${VAR} lib_${LIBRARY}_shared)
    elseif(VARIANT STREQUAL "static")
      list(APPEND ${VAR} lib_${LIBRARY}_static)
    elseif(NOT LIBRARY STREQUAL "shared" AND NOT LIBRARY STREQUAL "static")
      list(APPEND ${VAR} ${LIBRARY})
    endif()
    set(VARIANT ${LIBRARY})
  endforeach(LIBRARY)
endmacro(PURPLE_EXPAND_LIBRARIES VAR)
