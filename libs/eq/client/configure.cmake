##
# Path : libs/configure.cmake
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2012 Stefan Eilemann <eile@eyescale.ch>
#               2010 Cedric Stalder <cedric.stalder@gmail.ch>
##

# versioning
if(NOT EQ_REVISION)
  set(EQ_REVISION 0)
endif()

update_file(${CMAKE_CURRENT_SOURCE_DIR}/version.in.h ${OUTPUT_INCLUDE_DIR}/eq/client/version.h)
install(FILES ${OUTPUT_INCLUDE_DIR}/eq/client/version.h DESTINATION include/eq/client COMPONENT eqdev)
# also install in old location for compatibility with old FindEqualizer scripts
install(FILES ${OUTPUT_INCLUDE_DIR}/eq/client/version.h DESTINATION include/eq COMPONENT eqdev)

# compile-time definitions
set(EQUALIZER_DEFINES)

if(CUDA_FOUND)
  list(APPEND EQUALIZER_DEFINES EQ_USE_CUDA)
endif(CUDA_FOUND)

if(MAGELLAN_FOUND)
  list(APPEND EQUALIZER_DEFINES EQ_USE_MAGELLAN)
endif(MAGELLAN_FOUND)

list(APPEND EQUALIZER_DEFINES GLEW_MX) # always define GLEW_MX
list(APPEND EQUALIZER_DEFINES GLEW_NO_GLU)
if(GLEW_MX_FOUND)
  list(APPEND EQUALIZER_DEFINES EQ_FOUND_GLEW_MX)
else()
  list(APPEND EQUALIZER_DEFINES GLEW_STATIC)
endif(GLEW_MX_FOUND)

if(WIN32) # maybe use BOOST_WINDOWS instead?
  list(APPEND EQUALIZER_DEFINES WGL)
  set(ARCH Win32)
endif(WIN32)

if(APPLE)
  set(ARCH Darwin)
endif(APPLE)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(ARCH Linux)
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

if(EQ_GLX_USED)
  list(APPEND EQUALIZER_DEFINES GLX)
endif(EQ_GLX_USED)

if(EQUALIZER_BUILD_2_0_API)
  list(APPEND EQUALIZER_DEFINES EQ_2_0_API)
endif()

if(EQ_GLEW_INTERNAL)
  list(APPEND EQUALIZER_DEFINES EQ_GLEW_INTERNAL)
endif()

set(DEFINES_FILE ${OUTPUT_INCLUDE_DIR}/eq/client/defines${ARCH}.h)
set(DEFINES_FILE_IN ${CMAKE_CURRENT_BINARY_DIR}/defines${ARCH}.h.in)

file(WRITE ${DEFINES_FILE_IN}
  "#ifndef EQ_DEFINES_${ARCH}_H\n"
  "#define EQ_DEFINES_${ARCH}_H\n\n"
  "#include <co/defines.h>\n\n"
  )

foreach(DEF ${EQUALIZER_DEFINES})
  file(APPEND ${DEFINES_FILE_IN}
    "#ifndef ${DEF}\n"
    "#  define ${DEF}\n"
    "#endif\n"
    )
endforeach(DEF ${EQUALIZER_DEFINES})

if(EQ_AGL_USED) # special case
  file(APPEND ${DEFINES_FILE_IN}
    "#ifndef __LP64__ // AGL not supported in 64 bit mode\n"
    "#  ifndef AGL\n"
    "#    define AGL\n"
    "#  endif\n"
    "#endif\n"
    )
endif(EQ_AGL_USED)

file(APPEND ${DEFINES_FILE_IN}
  "\n#endif /* EQ_DEFINES_${ARCH}_H */\n"
  )

update_file(${DEFINES_FILE_IN} ${DEFINES_FILE})
install(FILES ${DEFINES_FILE} DESTINATION include/eq/client COMPONENT eqdev)

