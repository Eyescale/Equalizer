# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2015 Stefan Eilemann <eile@eyescale.ch>
#                    2011 Maxim Makhinya <maxmah@gmail.com>

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

if(Equalizer_BINARY_DIR) # in-repository build
  configure_file(CMakeLists.txt ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt
    @ONLY)

  set(EQUALIZER_LIBRARIES Equalizer)
  set(EQUALIZER_ADMIN_LIBRARY EqualizerAdmin)
  set(EQUALIZER_SERVER_LIBRARY EqualizerServer)
  set(EQUALIZER_SEQUEL_LIBRARY Sequel)
else() # installed examples
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../CMake)
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)
  find_package(Equalizer @VERSION@ REQUIRED)
  include_directories(${EQUALIZER_INCLUDE_DIRS})

  include(configure)
  include(CMakeParseArguments)
  set(EQ_AGL_USED @EQ_AGL_USED@)
  set(EQ_GLX_USED @EQ_GLX_USED@)

  if(APPLE)
    include_directories(SYSTEM /usr/X11R6/include)
    if(EQ_AGL_USED)
      find_library(CARBON_LIBRARY Carbon)
    endif()
  endif(APPLE)
endif()

include(StringifyShaders)

function(EQ_ADD_EXAMPLE NAME)
  cmake_parse_arguments(THIS "" "" "SOURCES;HEADERS;SHADERS;LINK_LIBRARIES"
    ${ARGN})

  stringify_shaders(${THIS_SHADERS})

  set(THIS_TARGET ${NAME})
  set(THIS_INPUT)
  source_group(\\ FILES ${THIS_SOURCES} ${THIS_HEADERS} ${THIS_SHADERS})
  add_executable(${THIS_TARGET} MACOSX_BUNDLE ${THIS_SOURCES} ${THIS_HEADERS}
                 ${SHADER_SOURCES})
  target_link_libraries(${THIS_TARGET} ${THIS_LINK_LIBRARIES}
    Equalizer ${Boost_PROGRAM_OPTIONS_LIBRARY})

  set_target_properties(${THIS_TARGET} PROPERTIES OUTPUT_NAME ${NAME}
                        FOLDER "Examples")
  if(AGL) # set preference to 32 bit if AGL is available
    configure_file(${PROJECT_SOURCE_DIR}/CMake/MacOSXBundleInfo.plist.in
      ${CMAKE_CURRENT_BINARY_DIR}/MacOSXBundleInfo.plist COPYONLY)
    set_target_properties(${THIS_TARGET} PROPERTIES MACOSX_BUNDLE_INFO_PLIST
      ${CMAKE_CURRENT_BINARY_DIR}/MacOSXBundleInfo.plist)
  endif()
  add_dependencies(${THIS_TARGET} ${EQUALIZER_SERVER_LIBRARY}) # dlopen'ed

  install(TARGETS ${THIS_TARGET} DESTINATION bin COMPONENT apps)
  if (MSVC)
    install(FILES ${PROJECT_BINARY_DIR}/bin/Debug/${THIS_TARGET}.pdb
      DESTINATION bin COMPONENT dev CONFIGURATIONS Debug)
    install(FILES ${PROJECT_BINARY_DIR}/bin/RelWithDebInfo/${THIS_TARGET}.pdb
      DESTINATION bin COMPONENT dev CONFIGURATIONS RelWithDebInfo)
  endif()
  install(FILES CMakeLists.txt ${THIS_SOURCES} ${THIS_HEADERS} ${THIS_SHADERS}
    DESTINATION share/Equalizer/examples/${NAME} COMPONENT examples)
endfunction(EQ_ADD_EXAMPLE NAME)

include_directories(BEFORE include ${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DEQ_SYSTEM_INCLUDES) # get GL headers
add_definitions(-DBOOST_PROGRAM_OPTIONS_DYN_LINK) # Fix for windows and shared boost.
if(GLEW_MX_FOUND)
  include_directories(BEFORE SYSTEM ${GLEW_MX_INCLUDE_DIRS})
endif()
list(APPEND CPPCHECK_EXTRA_ARGS -I${CMAKE_CURRENT_SOURCE_DIR})

install(DIRECTORY configs DESTINATION share/Equalizer COMPONENT examples
  FILES_MATCHING PATTERN "*.eqc"  PATTERN "issue*" EXCLUDE)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt
        DESTINATION share/Equalizer/examples COMPONENT examples)
install(DIRECTORY include DESTINATION share/Equalizer/examples
        COMPONENT examples)
install(FILES ${CMAKE_SOURCE_DIR}/CMake/common/StringifyShaders.cmake
              ${CMAKE_SOURCE_DIR}/CMake/common/FindGLEW_MX.cmake
              ${PROJECT_SOURCE_DIR}/CMake/configure.cmake
              DESTINATION share/Equalizer/examples/CMake COMPONENT examples)

add_subdirectory(triply) # build the ply library universal

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
