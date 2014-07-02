# Copyright (c) 2014 Stefan.Eilemann@epfl.ch
# sets EQ_GL_LIBRARIES as a function of EQ_AGL/GLX/WGL_USED
#   find_package(OpenGL) and find_package(Equalizer) need to be done beforehand

set(EQ_GL_LIBRARIES)
if(APPLE)
  if(EQ_AGL_USED)
    list(APPEND EQ_GL_LIBRARIES ${OPENGL_LIBRARIES})
  endif()
  if(EQ_GLX_USED)
    list(APPEND EQ_GL_LIBRARIES GL)
  endif()
else()
  list(APPEND EQ_GL_LIBRARIES ${OPENGL_gl_LIBRARY})
endif()
