##
# Path : libs/configure.cmake
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>
#               2010 Cedric Stalder <cedric.stalder@gmail.ch>
##

set(EQUALIZER_DEFINES)

# always define GLEW_MX
list(APPEND EQUALIZER_DEFINES GLEW_MX)

if(CUDA_FOUND)
  list(APPEND EQUALIZER_DEFINES EQ_USE_CUDA)
endif(CUDA_FOUND)

# maybe use BOOST_WINDOWS instead?
if(WIN32)
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
install(FILES ${DEFINES_FILE} DESTINATION include/eq/ COMPONENT dev)
