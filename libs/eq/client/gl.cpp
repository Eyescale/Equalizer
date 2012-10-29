
/* Copyright (c) 2011-2012, Stefan Eilemann <eile@eyescale.ch>
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

#include "gl.h"

#include <eq/client/glException.h>
#include <lunchbox/debug.h>

namespace eq
{
void debugGLError( const std::string& when, const GLenum error,
                   const char* file, const int line )
{
    LBWARN << lunchbox::disableFlush << "Got " << glError( error ) << ' '
           << when << " in " << file << ':' << line << std::endl
           << lunchbox::backtrace << lunchbox::enableFlush << std::endl;
    throw GLException( error );
}

std::string glError( const GLenum error )
{
    switch( error )
    {
        case GL_INVALID_ENUM:
            return std::string( "GL_INVALID_ENUM" );
        case GL_INVALID_VALUE:
            return std::string( "GL_INVALID_VALUE" );
        case GL_INVALID_OPERATION:
            return std::string( "GL_INVALID_OPERATION" );
        case GL_STACK_OVERFLOW:
            return std::string( "GL_STACK_OVERFLOW" );
        case GL_STACK_UNDERFLOW:
            return std::string( "GL_STACK_UNDERFLOW" );
        case GL_OUT_OF_MEMORY:
            return std::string( "GL_OUT_OF_MEMORY" );
        default:
            break;
    }

    std::ostringstream os;
    os << "GL error 0x" << std::hex << error << std::dec;
    return os.str();
}

}
