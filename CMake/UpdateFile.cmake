
# Copyright (c) 2012 Stefan Eilemann <eile@eyescale.ch>

# similar to configure_file, but overwrites target only if content differs
function(UPDATE_FILE IN OUT)
  if(NOT EXISTS ${OUT})
    configure_file(${IN} ${OUT} @ONLY)
    return()
  endif()

  configure_file(${IN} ${OUT}.tmp)
  file(READ ${OUT} _old_contents @ONLY)
  file(READ ${OUT}.tmp _new_contents)
  if("${_old_contents}" STREQUAL "${_new_contents}")
    file(REMOVE ${OUT}.tmp)
  else()
    file(RENAME ${OUT}.tmp ${OUT})
  endif()
endfunction()