
##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>
#               2010 Cedric Stalder <cedric.stalder@gmail.com>
##
set(EQ_UTIL_PUBLIC_HEADERS
  ../util/accum.h
  ../util/accumBufferObject.h
  ../util/base.h
  ../util/bitmapFont.h
  ../util/frameBufferObject.h
  ../util/objectManager.h
  ../util/pixelBufferObject.h
  ../util/texture.h
  ../util/types.h
  )

set(EQ_UTIL_HEADERS
  ../util/gpuCompressor.h
  )

set(EQ_UTIL_SOURCES
  ../util/accum.cpp
  ../util/accumBufferObject.cpp
  ../util/bitmapFont.cpp
  ../util/bitmapFont.ipp
  ../util/frameBufferObject.cpp
  ../util/gpuCompressor.cpp
  ../util/objectManager.cpp
  ../util/objectManager.ipp
  ../util/pixelBufferObject.cpp
  ../util/texture.cpp
  )
