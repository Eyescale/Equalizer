##
# Copyright (c) 2010 Daniel Pfeifer, All rights reserved.
#
# This file is freely distributable without licensing fees and
# is provided without guarantee or warrantee expressed or implied.
# This file is -not- in the public domain.
##

macro(_PURPLE_PRECOMPILE_HEADER_GCC  NAME SOURCE_VAR)
  # TODO
endmacro(_PURPLE_PRECOMPILE_HEADER_GCC)

macro(_PURPLE_PRECOMPILE_HEADER_MSVC  NAME SOURCE_VAR)
  set(PCH_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_pch.hpp")
  set(PCH_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_pch.cpp")
  
  if(MSVC_IDE)
    set(PCH_BINARY "$(IntDir)/${NAME}.pch")
  else()
    set(PCH_BINARY "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.pch")
  endif()

  file(WRITE ${PCH_HEADER}.in "/* ${NAME} precompuled header file */\n\n")
  foreach(HEADER ${ARGN})
    if(HEADER MATCHES "^<.*>$")
	  file(APPEND ${PCH_HEADER}.in "#include ${HEADER}\n")
	else()
      get_filename_component(HEADER_ABS ${HEADER} ABSOLUTE)
      file(RELATIVE_PATH HEADER_REL ${CMAKE_CURRENT_BINARY_DIR} ${HEADER_ABS})
	  file(APPEND ${PCH_HEADER}.in "#include \"${HEADER_REL}\"\n")
	endif()
  endforeach(HEADER ${ARGN})
  configure_file(${PCH_HEADER}.in ${PCH_HEADER} COPYONLY)

  file(WRITE ${PCH_SOURCE}.in "#include \"${PCH_HEADER}\"\n")
  configure_file(${PCH_SOURCE}.in ${PCH_SOURCE} COPYONLY)

  set_source_files_properties(${PCH_SOURCE} PROPERTIES
    COMPILE_FLAGS "/Yc\"${PCH_HEADER}\" /Fp\"${PCH_BINARY}\""
    OBJECT_OUTPUTS "${PCH_BINARY}")

  set_source_files_properties(${${SOURCE_VAR}} PROPERTIES
    COMPILE_FLAGS "/Yu\"${PCH_HEADER}\" /FI\"${PCH_HEADER}\" /Fp\"${PCH_BINARY}\""
    OBJECT_DEPENDS "${PCH_BINARY}")

  list(APPEND ${SOURCE_VAR} ${PCH_SOURCE})
endmacro(_PURPLE_PRECOMPILE_HEADER_MSVC)

# Precompiled headers should be used purely as a way to improve
# compilation time, not to save the number of #include statements.
# If a source file needs to include some header, explicitly include
# it in the source file, even if the same header is included from
# the precompiled header. 

macro(PURPLE_PRECOMPILE_HEADER)
  if(MSVC)
    _purple_precompile_header_msvc(${ARGN})
  endif(MSVC)
  # TODO
endmacro(PURPLE_PRECOMPILE_HEADER)
