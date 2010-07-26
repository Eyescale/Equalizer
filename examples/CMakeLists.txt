# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

include_directories(include)

install(DIRECTORY configs DESTINATION shared/Equalizer COMPONENT data)

add_subdirectory(eqHello)
#add_subdirectory(eqNBody) #hasn't been tried to compile for a while i guess
add_subdirectory(eqPixelBench)
add_subdirectory(eqPly)
add_subdirectory(eVolve)
#add_subdirectory(osgScaleViewer) #version conflict?
