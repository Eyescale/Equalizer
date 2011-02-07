##
# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
##

# TODO: move ../glX* into this directory, fix link_libraries

purple_add_library(eq_glx ${EQ_LIB_VARIANT}
  HEADERS_PREFIX "eq/"
  SOURCES
    ../glXEventHandler.cpp
    ../glXMessagePump.cpp
    ../glXWindow.cpp
    ../glXPipe.cpp
    glxFactory.cpp
# LINK_LIBRARIES
#   Collage
#   shared eq_fabric
#   ${GLEW_LIBRARY}
#   ${CARBON_LIBRARY}
#   ${X11_LIBRARIES}
#   ${GL_LIBRARIES}
#   ${CUDA_LIBRARIES}
  )
