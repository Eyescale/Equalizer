
/* Copyright (c) 2011-2016, Stefan Eilemann <eile@eyescale.ch>
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
#  ifdef EQ_FOUND_GLEW_MX
#    include <GL/glew.h>
#    ifdef GLX
#      include <GL/glxew.h>
#    endif
#    ifdef WGL
#      include <GL/wglew.h>
#    endif
#  else
#    include <eq/GL/glew.h>
#    ifdef GLX
#      include <eq/GL/glxew.h>
#    endif
#    ifdef WGL
#      include <eq/GL/wglew.h>
#    endif
#  endif
#endif

#ifdef AGL
#  include <OpenGL/gl.h>
#else
#  include <eq/os.h>
#  include <GL/gl.h>
#endif

#ifndef GL_TEXTURE_RECTANGLE_ARB
#  define GL_TEXTURE_RECTANGLE_ARB 0x84F5
#endif
#ifndef GLX_RGBA_FLOAT_BIT
#  define GLX_RGBA_FLOAT_BIT GLX_RGBA_FLOAT_BIT_ARB
#  define GLX_RGBA_FLOAT_TYPE GLX_RGBA_FLOAT_TYPE_ARB
#endif

# define EQ_UNKNOWN_GL_ERROR 0x1 // GL error codes seem to start at 0x500
/** @endcond */

// Error-check macros
namespace eq
{
/** Output an error OpenGL in a human-readable form to LBWARN */
EQ_API void debugGLError( const std::string& when, const GLenum error,
                          const char* file, const int line );

/** @return the given error in human-readable form. */
EQ_API std::string glError( const GLenum error );
}

#ifdef NDEBUG
#  define EQ_GL_ERROR( when ) {}
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
