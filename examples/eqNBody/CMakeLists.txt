# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>

include_directories(SYSTEM ${CUDA_INCLUDE_DIRS})

# WAR bug in FindCUDA.cmake:
remove_definitions(${EQ_DEFINITIONS})

if(MSVC)
  set(CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:LIBC;LIBCMT;MSVCRT)
endif()

# CUDA 4.x doesn't support gcc compilers greater than 4.4...
include(CompilerVersion)
compiler_dumpversion(GCC_COMPILER_VERSION)
if( ${CUDA_VERSION} VERSION_GREATER 4 AND
    GCC_COMPILER_VERSION VERSION_GREATER 4.4)

  # This code snippet looks for gcc-4.4 and creates a sym link to it
  # in the build directory so nvcc can be told to search for the
  # compiler in that path Hint provided to avoid the symbolic link by
  # ccache to be get first
  find_program(GCC_4_4 gcc-4.4 HINTS /usr/bin)
  mark_as_advanced(GCC_4_4)
  if (NOT GCC_4_4)
    message(WARNING "Only gcc 4.4 is supported by CUDA 4.x. Please install "
      "your distribution packages for gcc 4.4.")
  else()
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${GCC_4_4}
      ${CMAKE_BINARY_DIR}/tmp/gcc)
    list(APPEND CUDA_NVCC_FLAGS --compiler-bindir ${CMAKE_BINARY_DIR}/tmp)
  endif()
endif()

cuda_compile(NBODY_FILES nbody.cu)

eq_add_example(eqNBody
  HEADERS
    channel.h
    client.h
    config.h
    configEvent.h
    controller.h
    frameData.h
    initData.h
    nbody.h
    node.h
    pipe.h
    render_particles.h
    sharedData.h
    sharedDataProxy.h
    window.h
  SOURCES
    ${NBODY_FILES}
    nbody.cu
    channel.cpp
    client.cpp
    config.cpp
    controller.cpp
    frameData.cpp
    initData.cpp
    main.cpp
    node.cpp
    pipe.cpp
    render_particles.cpp
    sharedData.cpp
    sharedDataProxy.cpp
    window.cpp
  LINK_LIBRARIES
    ${CUDA_LIBRARIES}
  )

install(FILES nbody_kernel.cu ${CMAKE_SOURCE_DIR}/CMake/CompilerVersion.cmake
  DESTINATION share/Equalizer/examples/eqNBody COMPONENT examples)
