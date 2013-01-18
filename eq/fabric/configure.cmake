
# Copyright (c) 2012-2013 Stefan Eilemann <eile@eyescale.ch>

# Compile definitions
set(EQUALIZER_DEFINES)

if(EQ_BIG_ENDIAN)
  list(APPEND EQUALIZER_DEFINES EQ_BIG_ENDIAN)
else()
  list(APPEND EQUALIZER_DEFINES EQ_LITTLE_ENDIAN)
endif()

if(EQUALIZER_BUILD_2_0_API)
  list(APPEND EQUALIZER_DEFINES EQ_2_0_API)
else()
  list(APPEND EQUALIZER_DEFINES EQ_1_0_API)
endif()

if(WIN32)
  set(ARCH Win32)
endif(WIN32)
if(APPLE)
  set(ARCH Darwin)
endif(APPLE)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(ARCH Linux)
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

# Write defines file
set(DEFINES_FILE ${OUTPUT_INCLUDE_DIR}/eq/fabric/defines${ARCH}.h)
set(DEFINES_FILE_IN ${CMAKE_CURRENT_BINARY_DIR}/defines${ARCH}.h.in)

file(WRITE ${DEFINES_FILE_IN}
  "#ifndef EQFABRIC_DEFINES_${ARCH}_H\n"
  "#define EQFABRIC_DEFINES_${ARCH}_H\n\n"
  )

foreach(DEF ${EQUALIZER_DEFINES})
  file(APPEND ${DEFINES_FILE_IN}
    "#ifndef ${DEF}\n"
    "#  define ${DEF}\n"
    "#endif\n"
    )
endforeach(DEF ${EQUALIZER_DEFINES})

file(APPEND ${DEFINES_FILE_IN}
  "\n#endif /* EQFABRIC_DEFINES_${ARCH}_H */\n"
  )

update_file(${DEFINES_FILE_IN} ${DEFINES_FILE})
install(FILES ${DEFINES_FILE} DESTINATION include/eq/fabric COMPONENT dev)
