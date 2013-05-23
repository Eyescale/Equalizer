/**
 * ivs - interactive volume splatter
 *
 * @author Philipp Schlegel (schlegel@ifi.uzh.ch)
 **/

#ifndef IVS_SYSTEM_OPENGL_H
#define IVS_SYSTEM_OPENGL_H

#include "System/Config.h"

// always use glew for the windows environment
#ifndef GLEW
#  ifdef IVS_WIN32
#    include <windows.h>
#    define GLEW
#  endif
#endif

// Apple uses OpenGL directory
#ifdef IVS_MAC
#  ifdef GLEW
#    include <OpenGL/glew.h>
#  else
#    include <OpenGL/OpenGL.h>
#    include <OpenGL/gl.h>
#    include <OpenGL/glext.h>
#    include <OpenGL/glu.h>
#  endif
// use glew on windows and unix systems
#elif defined(GLEW)
#  include <GL/glew.h>
#else
// use the extensions on unix systems
#  ifndef GL_GLEXT_PROTOTYPES
#    define GL_GLEXT_PROTOTYPES 1
#  endif
#  include <GL/gl.h>
#  include <GL/glext.h>
#  include <GL/glu.h>
#endif

#endif
