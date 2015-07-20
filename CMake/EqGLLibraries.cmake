# Copyright (c) 2014-2015 Stefan.Eilemann@epfl.ch
#
# find_package(OpenGL) and find_package(Equalizer) need to be done beforehand
#
# Sets:
# - EQ_GL_LIBRARIES as a function of EQ_AGL/GLX/WGL_USED
# - GLEW_MX_LIBRARIES to the static Eq-supplied one if GLEW_MX was not found

set(EQ_GL_LIBRARIES)

if(APPLE)
  if(EQ_AGL_USED OR EQ_QT_USED)
    list(APPEND EQ_GL_LIBRARIES ${OPENGL_LIBRARIES})
  elseif(EQ_GLX_USED)
    list(APPEND EQ_GL_LIBRARIES GL)
    # see FindOpenGL.cmake...
    find_library(OPENGL_glu_LIBRARY GLU PATHS /opt/X11/lib /usr/X11R6/lib)
  endif()
else()
  list(APPEND EQ_GL_LIBRARIES ${OPENGL_gl_LIBRARY})
endif()

if(EQ_GLX_USED)
  # The GLEW_MX finder checks for glxew symbols only if X11_FOUND is set:
  find_package(X11 QUIET REQUIRED)
endif()

if(NOT GLEW_MX_FOUND)
  find_package(GLEW_MX QUIET)
  if(NOT GLEW_MX_FOUND)
    set(GLEW_MX_LIBRARIES ${GLEW_MX_EQUALIZER_LIBRARY})
    # eile WAR: In Livre, ${GLEW_MX_EQUALIZER_LIBRARY} is lost when
    # including this file, that is, it is set before the include but
    # no longer in this file. I could not figure out why, so simply
    # workaround it here:
    if(NOT GLEW_MX_LIBRARIES)
      set(GLEW_MX_LIBRARIES GLEW_MX_Equalizer)
    endif()
    add_definitions(-DGLEW_BUILD=1 -DGLEW_MX=1)
  endif()
endif()
