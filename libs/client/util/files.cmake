##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
##

# TODO: resolve two-way dependency between client and util!

set(UTIL_HEADERS
  util/accum.h
  util/accumBufferObject.h
  util/base.h
  util/bitmapFont.h
  util/frameBufferObject.h
  util/objectManager.h
  util/objectManager.ipp
  util/texture.h
  util/types.h
  )

set(UTIL_SOURCES
  util/accum.cpp
  util/accumBufferObject.cpp
  util/bitmapFont.cpp
  util/gpuCompressor.cpp
  util/frameBufferObject.cpp
  util/objectManager.cpp
  util/texture.cpp
  )
