##
# Path : libs/configure.cmake
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>
#               2010 Cedric Stalder <cedric.stalder@gmail.ch>
##

# versioning
if(NOT EQ_REVISION)
  set(EQ_REVISION 0)
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.in.h ${EQ_INCLUDE_DIR}/eq/version.h)
install(FILES ${EQ_INCLUDE_DIR}/eq/version.h DESTINATION include/eq/ COMPONENT eqdev)

# compile-time definitions
set(EQUALIZER_DEFINES)

list(APPEND EQUALIZER_DEFINES GLEW_MX) # always define GLEW_MX

if(CUDA_FOUND)
  list(APPEND EQUALIZER_DEFINES EQ_USE_CUDA)
endif(CUDA_FOUND)

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

if(MSVC)
  list(APPEND EQUALIZER_DEFINES EQ_USE_MAGELLAN)
endif(MSVC)

if(EQ_AGL_USED)
  list(APPEND EQUALIZER_DEFINES AGL)
endif(EQ_AGL_USED)

if(EQ_GLX_USED)
  list(APPEND EQUALIZER_DEFINES GLX)
endif(EQ_GLX_USED)

if(EQ_GLEW_INTERNAL)
  list(APPEND EQUALIZER_DEFINES EQ_GLEW_INTERNAL)
endif()

set(DEFINES_FILE ${EQ_INCLUDE_DIR}/eq/defines${ARCH}.h)
set(DEFINES_FILE_IN ${CMAKE_CURRENT_BINARY_DIR}/defines.h.in)

file(WRITE ${DEFINES_FILE_IN}
  "#ifndef EQ_DEFINES_${ARCH}_H\n"
  "#define EQ_DEFINES_${ARCH}_H\n\n"
  "#include <co/base/defines.h>\n\n"
  )

foreach(DEF ${EQUALIZER_DEFINES})
  file(APPEND ${DEFINES_FILE_IN}
    "#ifndef ${DEF}\n"
    "#  define ${DEF}\n"
    "#endif\n"
    )
endforeach(DEF ${EQUALIZER_DEFINES})

file(APPEND ${DEFINES_FILE_IN}
  "\n#endif /* EQ_DEFINES_${ARCH}_H */\n"
  )

configure_file(${DEFINES_FILE_IN} ${DEFINES_FILE} COPYONLY)
install(FILES ${DEFINES_FILE} DESTINATION include/eq/ COMPONENT eqdev)

