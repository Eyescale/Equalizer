##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
##

# TODO: resolve two-way dependancy between client and util!

set(UTIL_HEADERS
  accum.h
  accumBufferObject.h
  base.h
  bitmapFont.h
  frameBufferObject.h
  objectManager.h
  texture.h
  types.h
  )

set(UTIL_SOURCES
  accum.cpp
  accumBufferObject.cpp
  bitmapFont.cpp
  gpuCompressor.cpp
  frameBufferObject.cpp
  objectManager.cpp
  texture.cpp
  )

purple_forward_headers("eq/util/" ${UTIL_HEADERS})

