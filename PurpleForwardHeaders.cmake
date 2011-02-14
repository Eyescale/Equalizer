##
# Copyright (c) 2010-2010 Daniel Pfeifer, All rights reserved.
#
# This file is freely distributable without licensing fees and
# is provided without guarantee or warrantee expressed or implied.
# This file is -not- in the public domain.
##

# On Windows, symlinks are available since Vista, but they require the
# /Create Symbolic Link/ privilege, which only administrators have by default.
if(CMAKE_HOST_WIN32 AND NOT DEFINED MKLINK_WORKING)
  set(test_file ${CMAKE_CURRENT_BINARY_DIR}/symlinktest)
  file(TO_NATIVE_PATH ${CMAKE_CURRENT_LIST_DIR} file)
  file(TO_NATIVE_PATH ${test_file} target)
  execute_process(COMMAND cmd /C mklink ${target} ${file})
  if(EXISTS ${test_file})
    set(MKLINK_WORKING TRUE CACHE INTERNAL "")
  else(EXISTS ${test_file})
    set(MKLINK_WORKING FALSE CACHE INTERNAL "")
    message(STATUS "Symlinks are NOT supported.")
  endif(EXISTS ${test_file})
endif(CMAKE_HOST_WIN32 AND NOT DEFINED MKLINK_WORKING)

# Make a header file available from another path.
#
#   purple_forward_file(<file> <target>)
#
# Where <file> is a path to an existing file that you want to include as if
# it were located at <target>.
#
# This function creates symlinks where available. As a fallback it simply creates
# a file at the target position that [c++] `#include`s the appropriate file.
function(purple_forward_file file target)
  if(EXISTS "${target}")
    return()
  endif()

  get_filename_component(directory "${target}" PATH)
  file(MAKE_DIRECTORY "${directory}")

  if(NOT CMAKE_HOST_WIN32)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${file}" "${target}")
  elseif(MKLINK_WORKING)
    file(TO_NATIVE_PATH "${file}" file)
    file(TO_NATIVE_PATH "${target}" target)
    execute_process(COMMAND cmd /C mklink "${target}" "${file}")
  else()
    # create forwarding header
    file(WRITE "${target}" "#include \"${file}\"\n")
  endif()
endfunction(purple_forward_file)

# create forwarding headers
function(PURPLE_FORWARD_HEADERS PREFIX)
  set(FWD_PREFIX "${CMAKE_BINARY_DIR}/include/${PREFIX}")
  foreach(file ${ARGN})
    get_filename_component(absolute "${file}" ABSOLUTE)
    file(RELATIVE_PATH relative "${CMAKE_CURRENT_SOURCE_DIR}" "${absolute}")
    set(fwdfile "${FWD_PREFIX}${relative}")
    purple_forward_file("${absolute}" "${fwdfile}")
  endforeach(file)
endfunction(PURPLE_FORWARD_HEADERS)
