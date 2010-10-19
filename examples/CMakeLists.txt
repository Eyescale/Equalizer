# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>

include_directories(include)

install(DIRECTORY configs
  DESTINATION share/Equalizer
  COMPONENT data
  PATTERN ".svn" EXCLUDE
  )

install(DIRECTORY include
  DESTINATION share/Equalizer/examples
  COMPONENT examples
  PATTERN ".svn" EXCLUDE
  )

add_subdirectory(eqHello)
add_subdirectory(eqNBody)
add_subdirectory(eqPixelBench)
add_subdirectory(eqPly)
add_subdirectory(eVolve)
add_subdirectory(osgScaleViewer)
