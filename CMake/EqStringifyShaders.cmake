##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
##

macro(EQ_STRINGIFY_SHADERS SOURCES)
  foreach(FILE ${ARGN})
    set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})
    set(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${FILE})
    set(OUTPUT_FILES ${OUTPUT}.h ${OUTPUT}.cpp)
    add_custom_command(OUTPUT ${OUTPUT_FILES}
      COMMAND ${CMAKE_COMMAND} -DEQ_STRINGIFY_SHADERS_PROCESSING_MODE=ON
        -DINPUT="${INPUT}" -DOUTPUT="${OUTPUT}"
        -P ${CMAKE_SOURCE_DIR}/CMake/EqStringifyShaders.cmake
      DEPENDS ${INPUT}
      )
    list(APPEND ${SOURCES} ${OUTPUT_FILES})
  endforeach(FILE ${ARGN})
endmacro(EQ_STRINGIFY_SHADERS SOURCES)

if(NOT EQ_STRINGIFY_SHADERS_PROCESSING_MODE)
  return()
endif(NOT EQ_STRINGIFY_SHADERS_PROCESSING_MODE)

#

get_filename_component(FILENAME ${INPUT} NAME)
string(REGEX REPLACE "[.]" "_" NAME ${FILENAME})

file(STRINGS ${INPUT} LINES)

file(WRITE ${OUTPUT}.h
  "/* Generated file, do not edit! */\n\n"
  "extern char const* const ${NAME};\n"
  )

file(WRITE ${OUTPUT}.cpp
  "/* Generated file, do not edit! */\n\n"
  "#include \"${FILENAME}.h\"\n\n"
  "char const* const ${NAME} = \n"
  )

foreach(LINE ${LINES})
  string(REPLACE "\"" "\\\"" LINE "${LINE}")
  file(APPEND ${OUTPUT}.cpp "   \"${LINE}\\n\"\n")
endforeach(LINE)

file(APPEND ${OUTPUT}.cpp "   ;\n")
