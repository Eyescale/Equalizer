
/* Copyright (c) 2011, Stefan Eilemann <eile@eyescale.ch> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef EQ_GL_H
#define EQ_GL_H

/**
 * @file eq/gl.h
 * 
 * Includes OpenGL and GLEW headers.
 *
 * Define EQ_IGNORE_GLEW before including any Equalizer header if you have
 * trouble with your system-installed OpenGL header and do not need GLEW.
 */

#include <eq/api.h>

#include <string>

/** @cond IGNORE */
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#ifndef EQ_IGNORE_GLEW
#  ifdef EQ_GLEW_INTERNAL
#    include <eq/GL/glew.h>
#    ifdef GLX
#      include <eq/GL/glxew.h>
#    endif
#    ifdef WGL
#      include <eq/GL/wglew.h>
#    endif
#  else
#    include <GL/glew.h>
#    ifdef GLX
#      include <GL/glxew.h>
#    endif
#    ifdef WGL
#      include <GL/wglew.h>
#    endif
#  endif
#endif

#ifdef AGL
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#ifndef GL_TEXTURE_RECTANGLE_ARB
#  define GL_TEXTURE_RECTANGLE_ARB 0x84F5
#endif
/** @endcond */

// Error-check macros
namespace eq
{
/** Output an error OpenGL in a human-readable form to EQWARN */
EQ_API void debugGLError( const std::string& when, const GLenum error, 
                          const char* file, const int line );
}

#ifdef NDEBUG
#  define EQ_GL_ERROR( when ) 
#  define EQ_GL_CALL( code ) { code; }
#else // NDEBUG
#  define EQ_GL_ERROR( when )                                           \
    {                                                                   \
        const GLenum eqGlError = glGetError();                          \
        if( eqGlError )                                                 \
            eq::debugGLError( when, eqGlError, __FILE__, __LINE__ );    \
    }

#  define EQ_GL_CALL( code )                              \
    {                                                     \
        EQ_GL_ERROR( std::string( "before " ) + #code );  \
        code;                                             \
        EQ_GL_ERROR( std::string( "after " ) + #code );   \
    }
#endif // NDEBUG

#endif // EQ_GL_H

