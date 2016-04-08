# Copyright (c) 2010-2016 Daniel Pfeifer <daniel@pfeifer-mail.de>
#                         Stefan Eilemann <eile@eyescale.ch>
#                         Maxim Makhinya <maxmah@gmail.com>

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

include_directories(BEFORE
 include ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DEQ_SYSTEM_INCLUDES) # get GL headers
add_definitions(-DBOOST_PROGRAM_OPTIONS_DYN_LINK) # Fix for windows and shared boost.
list(APPEND CPPCHECK_EXTRA_ARGS -I${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(triply) # ply library used by *Ply

if(APPLE)
  # filter out -arch settings from environment (MacPorts...)
  set(ALL_OSX_ARCHITECTURES i386 x86_64)
  foreach(ARCH ${ALL_OSX_ARCHITECTURES})
    list(FIND CMAKE_OSX_ARCHITECTURES ${ARCH} INDEX)
    if(INDEX EQUAL -1) # Arch not enabled
      string(REGEX REPLACE "-arch ${ARCH}" "" CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS}")
      string(REGEX REPLACE "-arch ${ARCH}" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
      string(REGEX REPLACE "-arch ${ARCH}" "" CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS}")
    endif()
  endforeach()
endif()

add_subdirectory(eVolve)
add_subdirectory(eqAsync)
add_subdirectory(eqCPU)
add_subdirectory(eqHello)
add_subdirectory(eqPixelBench)
add_subdirectory(eqPly)
add_subdirectory(seqPly)
if(CUDA_FOUND)
  add_subdirectory(eqNBody)
endif()
if(OPENSCENEGRAPH_FOUND)
  add_subdirectory(osgScaleViewer)
endif()

install(DIRECTORY configs DESTINATION share/Equalizer COMPONENT examples
  FILES_MATCHING PATTERN "*.eqc"  PATTERN "issue*" EXCLUDE)
install(DIRECTORY include DESTINATION share/Equalizer/examples
        COMPONENT examples)
