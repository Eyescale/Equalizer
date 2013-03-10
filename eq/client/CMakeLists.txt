
# Path : libs/client/CMakeLists.txt
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2012 Stefan Eilemann <eile@eyescale.ch>
#               2011 Cedric Stalder <cedric.stalder@gmail.ch>

if(EQUALIZER_USE_MAGELLAN)
  find_package(MAGELLAN)
endif()

# TODO: resolve two-way dependency between client and util!
include(InstallFiles)
include(configure.cmake)
include(files.cmake)
include(../util/files.cmake)
include(compressor/files.cmake)

add_definitions(-DEQ_PLUGIN_BUILD)

set(EQ_LIBRARIES ${PTHREAD_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARY}
  EqualizerFabric)
if(MAGELLAN_FOUND)
  include_directories(${MAGELLAN_INCLUDE_DIR})
  list(APPEND EQ_LIBRARIES ${MAGELLAN_LIBRARY})
endif()
if(GLSTATS_FOUND)
  list(APPEND EQ_LIBRARIES ${GLSTATS_LIBRARIES})
endif()

if(HWLOC_GL_FOUND)
  include_directories(${HWLOC_INCLUDE_DIRS})
  list(APPEND EQ_LIBRARIES ${HWLOC_LIBRARIES})
endif()

if(SAGE_FOUND)
  include_directories(${SAGE_INCLUDE_DIRS})
  list(APPEND EQ_LIBRARIES ${SAGE_LIBRARIES})
endif()

if(OpenCV_FOUND)
  include_directories(${OpenCV_INCLUDE_DIRS})
  list(APPEND EQ_LIBRARIES ${OpenCV_LIBRARIES})
endif()

if(APPLE)
  if(EQ_AGL_USED)
    find_library(CARBON_LIBRARY Carbon)
    list(APPEND EQ_LIBRARIES ${OPENGL_LIBRARIES})
  endif()
  if(EQ_GLX_USED)
    list(APPEND EQ_LIBRARIES GL)
  endif()
else()
  list(APPEND EQ_LIBRARIES ${OPENGL_gl_LIBRARY})
endif()
if(CUDA_FOUND)
  list(APPEND EQ_LIBRARIES ${CUDA_LIBRARIES})
endif()

if(EQ_AGL_USED)
  list(APPEND EQ_LIBRARIES ${CARBON_LIBRARY})
endif()
if(EQ_GLX_USED)
  list(APPEND EQ_LIBRARIES ${X11_LIBRARIES})
endif()

source_group(fabric REGULAR_EXPRESSION .*)
source_group(\\ FILES CMakeLists.txt)
source_group(plugin FILES ${PLUGIN_HEADERS} )
source_group(util FILES ${UTIL_HEADERS} ${UTIL_SOURCES})
source_group(downloader FILES ${EQ_COMPRESSOR_HEADERS} ${EQ_COMPRESSOR_SOURCES})
source_group(client FILES eq.h ${CLIENT_HEADERS} ${CLIENT_SOURCES})

source_group(agl FILES ${AGL_HEADERS} ${AGL_SOURCES})
source_group(glx FILES ${GLX_HEADERS} ${GLX_SOURCES})
source_group(wgl FILES ${WGL_HEADERS} ${WGL_SOURCES})
set_source_files_properties(${AGL_SOURCES}
  PROPERTIES COMPILE_FLAGS "-Wno-multichar")

add_library(Equalizer SHARED ${CLIENT_HEADERS} ${UTIL_HEADERS}
  ${CLIENT_SOURCES} ${UTIL_SOURCES} ${EQ_COMPRESSOR_SOURCES})
target_link_libraries(Equalizer ${GLEW_LIBRARY} ${EQ_LIBRARIES})
set_target_properties(Equalizer
  PROPERTIES VERSION ${VERSION} SOVERSION ${VERSION_ABI})

install_files(include/eq/client FILES ${CLIENT_HEADERS}  COMPONENT eqdev)
install_files(include/eq/util FILES ${UTIL_HEADERS} COMPONENT eqdev)
install(TARGETS Equalizer
        ARCHIVE DESTINATION lib COMPONENT eqdev
        RUNTIME DESTINATION bin COMPONENT eqlib
        LIBRARY DESTINATION lib COMPONENT eqlib)
install(FILES ${OPTIONS_CMAKE} DESTINATION ${CMAKE_MODULE_INSTALL_PATH} COMPONENT dev)

option(EQUALIZER_OLD_CLIENT_LAYOUT
  "Install header files as in v1.0 directory layout" OFF)
if(EQUALIZER_OLD_CLIENT_LAYOUT)
  install(FILES ${CLIENT_HEADERS} DESTINATION include/eq/ COMPONENT eqdev)
endif()
