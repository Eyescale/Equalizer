
# Copyright (c) 2010 Daniel Pfeifer, All rights reserved.
#               2012 Stefan Eilemann <eile@eyescale.ch>
#
# This file is freely distributable without licensing fees and
# is provided without guarantee or warrantee expressed or implied.
# This file is -not- in the public domain.

include(UpdateFile)

macro(_PURPLE_PCH_FILES PCH_HEADER PCH_SOURCE NAME)
  set(${PCH_HEADER} "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_pch.hxx")
  set(${PCH_SOURCE} "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_pch.cxx")
endmacro(_PURPLE_PCH_FILES PCH_HEADER PCH_SOURCE NAME)

macro(_PURPLE_PRECOMPILE_HEADER_GCC NAME TARGET)
  _purple_pch_files(PCH_INPUT PCH_SOURCE ${NAME})

  set(PCH_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_pch.hxx")
  set(PCH_BINARY "${PCH_HEADER}.gch")

  update_file(${PCH_INPUT} ${PCH_HEADER})

  string(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" VARIANT)
  set(COMPILE_FLAGS ${${VARIANT}} )

  #get_target_property(TARGET_TYPE ${TARGET} TYPE)
  #if(${TARGET_TYPE} STREQUAL SHARED_LIBRARY)
  #  list(APPEND COMPILE_FLAGS "-fPIC")
  #endif(${TARGET_TYPE} STREQUAL SHARED_LIBRARY)

  get_directory_property(INC_DIRS INCLUDE_DIRECTORIES)
  foreach(DIR ${INC_DIRS})
    list(APPEND COMPILE_FLAGS "-I${DIR}")
  endforeach(DIR)

  get_directory_property(DIRECTORY_DEFS DEFINITIONS)
  list(APPEND COMPILE_FLAGS ${DIRECTORY_DEFS} ${CMAKE_CXX_FLAGS})

  separate_arguments(COMPILE_FLAGS)

  add_custom_command(OUTPUT ${PCH_BINARY} 	
    COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} ${COMPILE_FLAGS} -x c++-header -o ${PCH_BINARY} ${PCH_HEADER} 
    )

  add_custom_target(${TARGET}_pch DEPENDS ${PCH_BINARY})
  add_dependencies(${TARGET} ${TARGET}_pch)
  
  get_target_property(OLD_FLAGS ${TARGET} COMPILE_FLAGS)
  if(${OLD_FLAGS} MATCHES NOTFOUND)
    set(OLD_FLAGS "")
  endif(${OLD_FLAGS} MATCHES NOTFOUND)

  set_target_properties(${TARGET} PROPERTIES
    COMPILE_FLAGS "${OLD_FLAGS} -include ${PCH_HEADER}"
    )
endmacro(_PURPLE_PRECOMPILE_HEADER_GCC)

macro(_PURPLE_PRECOMPILE_HEADER_MSVC NAME TARGET)
  _purple_pch_files(PCH_HEADER PCH_SOURCE ${NAME})

  get_target_property(OLD_FLAGS ${TARGET} COMPILE_FLAGS)
  if(${OLD_FLAGS} MATCHES NOTFOUND)
    set(OLD_FLAGS "")
  endif(${OLD_FLAGS} MATCHES NOTFOUND)

  set_target_properties(${TARGET} PROPERTIES
    COMPILE_FLAGS "${OLD_FLAGS} /Yu\"${PCH_HEADER}\" /FI\"${PCH_HEADER}\""
    )
  set_source_files_properties(${PCH_SOURCE} PROPERTIES
    COMPILE_FLAGS "${OLD_FLAGS} /Yc\"${PCH_HEADER}\""
    )
endmacro(_PURPLE_PRECOMPILE_HEADER_MSVC)

# Precompiled headers should be used purely as a way to improve
# compilation time, not to save the number of #include statements.
# If a source file needs to include some header, explicitly include
# it in the source file, even if the same header is included from
# the precompiled header. 

macro(PURPLE_PCH_PREPARE NAME SOURCE_VAR)
  _purple_pch_files(PCH_HEADER PCH_SOURCE ${NAME})

  file(WRITE ${PCH_HEADER}.in "/* ${NAME} precompiled header file */\n\n")
  foreach(HEADER ${ARGN})
    if(HEADER MATCHES "^<.*>$")
      file(APPEND ${PCH_HEADER}.in "#include ${HEADER}\n")
    else()
      get_filename_component(HEADER_ABS ${HEADER} ABSOLUTE)
      file(RELATIVE_PATH HEADER_REL ${CMAKE_CURRENT_BINARY_DIR} ${HEADER_ABS})
      file(APPEND ${PCH_HEADER}.in "#include \"${HEADER_REL}\"\n")
    endif()
  endforeach(HEADER ${ARGN})
  update_file(${PCH_HEADER}.in ${PCH_HEADER} COPYONLY)

  file(WRITE ${PCH_SOURCE}.in "#include \"${PCH_HEADER}\"\n")
  update_file(${PCH_SOURCE}.in ${PCH_SOURCE} COPYONLY)

  if(MSVC)
    list(APPEND ${SOURCE_VAR} ${PCH_SOURCE})
  endif(MSVC)
endmacro(PURPLE_PCH_PREPARE)

macro(_PURPLE_PCH_TARGET_USE)
  if(MSVC)
    _purple_precompile_header_msvc(${ARGN})
  elseif(CMAKE_COMPILER_IS_GNUCXX)
    _purple_precompile_header_gcc(${ARGN})
  endif()
endmacro(_PURPLE_PCH_TARGET_USE)

macro(PURPLE_PCH_USE NAME)
  foreach(TARGET ${ARGN})
    _purple_pch_target_use(${NAME} ${TARGET}) 
  endforeach(TARGET)
endmacro(PURPLE_PCH_USE)
