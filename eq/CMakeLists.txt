# Copyright (c) 2011-2016 Stefan Eilemann <eile@eyescale.ch>

include_directories(SYSTEM ${OPENGL_INCLUDE_DIR} ${GLEW_MX_INCLUDE_DIRS})

list(APPEND CPPCHECK_EXTRA_ARGS -DEQ_API=)

add_definitions(-DBOOST_PROGRAM_OPTIONS_DYN_LINK)

set(EQUALIZER_PUBLIC_HEADERS
  agl/eventHandler.h
  agl/messagePump.h
  agl/pipe.h
  agl/types.h
  agl/window.h
  base.h
  canvas.h
  channel.h
  channelStatistics.h
  client.h
  commandQueue.h
  compositor.h
  config.h
  configStatistics.h
  eq.h
  error.h
  eventHandler.h
  eventICommand.h
  exception.h
  eye.h
  frame.h
  frameData.h
  gl.h
  glException.h
  glWindow.h
  global.h
  glx/eventHandler.h
  glx/messagePump.h
  glx/pipe.h
  glx/types.h
  glx/window.h
  image.h
  imageOp.h
  init.h
  layout.h
  log.h
  messagePump.h
  node.h
  nodeFactory.h
  notifierInterface.h
  observer.h
  os.h
  pipe.h
  pixelData.h
  resultImageListener.h
  segment.h
  server.h
  statisticSampler.h
  system.h
  systemPipe.h
  systemWindow.h
  types.h
  util/accum.h
  util/accumBufferObject.h
  util/base.h
  util/bitmapFont.h
  util/frameBufferObject.h
  util/objectManager.h
  util/pixelBufferObject.h
  util/shader.h
  util/texture.h
  util/types.h
  view.h
  visitorResult.h
  wgl/eventHandler.h
  wgl/messagePump.h
  wgl/pipe.h
  wgl/types.h
  wgl/window.h
  window.h
  windowSettings.h
  windowStatistics.h
  windowSystem.h
  x11/window.h
  zoomFilter.h
  )

set(EQUALIZER_HEADERS
  agl/windowSystem.h
  detail/fileFrameWriter.h
  detail/statsRenderer.h
  exitVisitor.h
  glx/windowSystem.h
  half.h
  initVisitor.h
  transferFinder.h
  wgl/windowSystem.h
  )

set(EQUALIZER_SOURCES
  util/accum.cpp
  util/accumBufferObject.cpp
  util/bitmapFont.cpp
  util/frameBufferObject.cpp
  util/objectManager.cpp
  util/pixelBufferObject.cpp
  util/shader.cpp
  util/texture.cpp
  canvas.cpp
  channel.cpp
  channelStatistics.cpp
  client.cpp
  commandQueue.cpp
  compositor.cpp
  config.cpp
  configStatistics.cpp
  detail/channel.ipp
  detail/fileFrameWriter.cpp
  eventHandler.cpp
  eventICommand.cpp
  frame.cpp
  frameData.cpp
  gl.cpp
  glException.cpp
  glWindow.cpp
  global.cpp
  half.cpp
  image.cpp
  imageOp.cpp
  init.cpp
  jitter.cpp
  layout.cpp
  node.cpp
  nodeFactory.cpp
  nodeStatistics.cpp
  observer.cpp
  pipe.cpp
  pipeStatistics.cpp
  pixelData.cpp
  roiEmptySpaceFinder.cpp
  roiFinder.cpp
  roiTracker.cpp
  segment.cpp
  server.cpp
  systemPipe.cpp
  systemWindow.cpp
  view.cpp
  window.cpp
  windowSettings.cpp
  windowStatistics.cpp
  windowSystem.cpp
  worker.cpp
  )

set(EQUALIZER_LINK_LIBRARIES
  PUBLIC EqualizerFabric ${GLEW_LIBRARY} ${EQ_GL_LIBRARIES}
  PRIVATE EqualizerServer Pression ${PTHREAD_LIBRARIES}
          ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_FILESYSTEM_LIBRARY}
          ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})

if(TARGET Deflect)
  list(APPEND EQUALIZER_SOURCES
    deflect/connection.h
    deflect/eventHandler.cpp
    deflect/eventHandler.h
    deflect/proxy.cpp
    deflect/proxy.h
    )
  list(APPEND EQUALIZER_LINK_LIBRARIES Deflect)
endif()

if(HWSD_FOUND)
  list(APPEND EQUALIZER_LINK_LIBRARIES hwsd)
endif()

if(OPENSCENEGRAPH_FOUND)
  list(APPEND EQUALIZER_LINK_LIBRARIES ${OPENSCENEGRAPH_LIBRARIES})
endif()

if(EQ_AGL_USED)
  set(AGL_SOURCES
    agl/eventHandler.cpp
    agl/messagePump.cpp
    agl/window.cpp
    agl/pipe.cpp
    )
  list(APPEND EQUALIZER_SOURCES ${AGL_SOURCES})
  set_source_files_properties(${AGL_SOURCES}
    PROPERTIES COMPILE_FLAGS "-Wno-multichar")
  find_library(CARBON_LIBRARY Carbon)
  find_library(AGL_LIBRARY AGL)
  list(APPEND EQUALIZER_LINK_LIBRARIES ${CARBON_LIBRARY} ${AGL_LIBRARY})
endif()

if(EQ_GLX_USED)
  list(APPEND EQUALIZER_SOURCES
    glx/eventHandler.cpp
    glx/messagePump.cpp
    glx/pipe.cpp
    glx/window.cpp
    glx/X11Connection.h
    glx/types.cpp
    x11/window.cpp
    )
  list(APPEND EQUALIZER_LINK_LIBRARIES ${X11_LIBRARIES})
endif()

if(WIN32)
  list(APPEND EQUALIZER_SOURCES
    wgl/eventHandler.cpp
    wgl/messagePump.cpp
    wgl/window.cpp
    wgl/pipe.cpp
    )
endif()

if(EQ_QT_USED)
  list(APPEND EQUALIZER_PUBLIC_HEADERS
    qt/types.h
    qt/window.h
    qt/shareContextWindow.h
    )
  list(APPEND EQUALIZER_HEADERS
    qt/windowFactory.h
    qt/windowSystem.h
    )
  list(APPEND EQUALIZER_SOURCES
    qt/detail/window.h
    qt/event.h
    qt/eventHandler.cpp
    qt/messagePump.cpp
    qt/pipe.cpp
    qt/window.cpp
    qt/windowFactory.cpp
    qt/windowSystem.cpp
    )
  list(APPEND EQUALIZER_LINK_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets)
endif()

if(OpenCV_FOUND)
  list(APPEND EQUALIZER_SOURCES detail/cvTracker.h detail/cvTracker.cpp)
  include_directories(${OpenCV_INCLUDE_DIRS})
  list(APPEND EQUALIZER_LINK_LIBRARIES ${OpenCV_LIBS})
  add_definitions(-DOPENCV_INSTALL_PATH=\"${OpenCV_INSTALL_PATH}\")
endif()

if(MAGELLAN_FOUND)
  include_directories(${MAGELLAN_INCLUDE_DIR})
  list(APPEND EQUALIZER_LINK_LIBRARIES ${MAGELLAN_LIBRARY})
endif()

if(GLSTATS_FOUND)
  list(APPEND EQUALIZER_LINK_LIBRARIES GLStats)
endif()

if(HWLOC_GL_FOUND)
  include_directories(${HWLOC_INCLUDE_DIRS})
  list(APPEND EQUALIZER_LINK_LIBRARIES ${HWLOC_LIBRARIES})
endif()

if(VRPN_FOUND)
  list(APPEND EQUALIZER_LINK_LIBRARIES ${VRPN_LIBRARIES})
endif()

set(EQUALIZER_OMIT_LIBRARY_HEADER ON)
common_library(Equalizer)
target_compile_definitions(Equalizer PRIVATE EQUALIZERFABRIC_SHARED_INL)
if(CMAKE_COMPILER_IS_CLANG)
  target_compile_options(Equalizer PUBLIC -Wno-overloaded-virtual)
endif()

add_subdirectory(compressor)
add_subdirectory(fabric)
add_subdirectory(server)
add_subdirectory(admin)
