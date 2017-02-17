# Copyright (c) 2017 Stefan.Eilemann@epfl.ch

include_directories(SYSTEM ${OPENGL_INCLUDE_DIR} ${GLEW_MX_INCLUDE_DIRS})

add_definitions(-DEQ_PLUGIN_BUILD)

set(EQUALIZERCOMPRESSOR_HEADERS
  compressor.h
  compressorReadDrawPixels.h
  compressorYUV.h
  )

set(EQUALIZERCOMPRESSOR_SOURCES
  compressor.cpp
  compressorReadDrawPixels.cpp
  compressorYUV.cpp
  )

set(EQUALIZERCOMPRESSOR_LINK_LIBRARIES PRIVATE Equalizer Pression)

common_library(EqualizerCompressor)
