# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

include_directories(include)

install(DIRECTORY configs
  DESTINATION share/Equalizer
  COMPONENT data
  PATTERN ".svn" EXCLUDE
  )

add_subdirectory(eqHello)
add_subdirectory(eqNBody)
add_subdirectory(eqPixelBench)
add_subdirectory(eqPly)
add_subdirectory(eVolve)
#add_subdirectory(osgScaleViewer)
