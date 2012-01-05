# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>
#                    2011 Maxim Makhinya <maxmah@gmail.com>

cmake_minimum_required(VERSION 2.6.4 FATAL_ERROR)

if(Equalizer_BINARY_DIR) # in-source build
  configure_file(CMakeLists.txt ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt @ONLY)

  set(EQUALIZER_LIBRARIES lib_Equalizer_shared)
  set(EQUALIZER_ADMIN_LIBRARY lib_EqualizerAdmin_shared)
  set(EQUALIZER_SERVER_LIBRARY lib_EqualizerServer_shared)
  set(EQUALIZER_SEQUEL_LIBRARY lib_Sequel_shared)
else()
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../CMake)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)
  find_package(Equalizer @VERSION@ REQUIRED)
  include_directories(${EQUALIZER_INCLUDE_DIRS})
  set(EQ_DEFINITIONS -DEQ_BUILD_DIR="${CMAKE_BINARY_DIR}/" -DEQ_SOURCE_DIR="${CMAKE_SOURCE_DIR}/")
  add_definitions(${EQ_DEFINITIONS})

  include(configure)
  include(ParseArguments)
  include(PurpleExpandLibraries)
  set(EQ_AGL_USED @EQ_AGL_USED@)
  set(EQ_GLX_USED @EQ_GLX_USED@)

  if(APPLE)
    link_directories(/usr/X11R6/lib)
    include_directories(SYSTEM /usr/X11R6/include)
    set(CMAKE_OSX_ARCHITECTURES "@EQUALIZER_OSX_ARCHITECTURES@")
  endif(APPLE)
  if(EQ_AGL_USED)
    find_library(CARBON_LIBRARY Carbon)
  endif()
endif()

include(EqStringifyShaders)

function(EQ_ADD_EXAMPLE NAME)
  parse_arguments(THIS "SOURCES;HEADERS;SHADERS;LINK_LIBRARIES" "" ${ARGN})

  eq_stringify_shaders(SHADERS ${THIS_SHADERS})

  set(THIS_TARGET ${NAME})
  set(THIS_INPUT)
  source_group(\\ FILES ${THIS_SOURCES} ${THIS_HEADERS} ${THIS_SHADERS})
  add_executable(${THIS_TARGET} MACOSX_BUNDLE ${THIS_SOURCES} ${THIS_HEADERS} ${SHADERS})

  purple_expand_libraries(LINK_LIBRARIES
                          ${EQUALIZER_LIBRARIES} ${THIS_LINK_LIBRARIES})
  target_link_libraries(${THIS_TARGET} ${LINK_LIBRARIES} ${OPENGL_LIBRARIES})

  set_target_properties(${THIS_TARGET} PROPERTIES OUTPUT_NAME ${NAME} FOLDER "Examples")
  add_dependencies(${THIS_TARGET} ${EQUALIZER_SERVER_LIBRARY}) # dlopen'ed

  install(TARGETS ${THIS_TARGET} DESTINATION bin COMPONENT apps)
  install(FILES CMakeLists.txt ${THIS_SOURCES} ${THIS_HEADERS} ${THIS_SHADERS}
    DESTINATION share/Equalizer/examples/${NAME} COMPONENT examples)
endfunction(EQ_ADD_EXAMPLE NAME)

include_directories(include)
if(GLEW_MX_FOUND)
  include_directories(BEFORE SYSTEM ${GLEW_MX_INCLUDE_DIRS})
endif()

install(DIRECTORY configs DESTINATION share/Equalizer
        COMPONENT data PATTERN ".svn" EXCLUDE)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt
        DESTINATION share/Equalizer/examples COMPONENT examples)
install(DIRECTORY include DESTINATION share/Equalizer/examples
        COMPONENT examples PATTERN ".svn" EXCLUDE)
install(FILES ${CMAKE_SOURCE_DIR}/CMake/ParseArguments.cmake
              ${CMAKE_SOURCE_DIR}/CMake/PurpleExpandLibraries.cmake
              ${CMAKE_SOURCE_DIR}/CMake/EqStringifyShaders.cmake
              ${CMAKE_SOURCE_DIR}/CMake/configure.cmake
              DESTINATION share/Equalizer/examples/CMake COMPONENT examples)

if(CUDA_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/eqNBody")
  add_subdirectory(eqNBody)
endif()
if(OSG_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/osgScaleViewer")
  add_subdirectory(osgScaleViewer)
endif()

add_subdirectory(eVolve)
add_subdirectory(eqAsync)
add_subdirectory(eqHello)
add_subdirectory(eqPixelBench)
add_subdirectory(eqPly)
add_subdirectory(seqPly)
