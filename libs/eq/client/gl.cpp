
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

#include "gl.h"

#include <lunchbox/debug.h>

namespace eq
{
void debugGLError( const std::string& when, const GLenum error, 
                   const char* file, const int line )
{                                                                 
    EQWARN << co::base::disableFlush << "Got ";
    switch( error )
    {
        case GL_INVALID_ENUM:
            EQWARN << "GL_INVALID_ENUM"; break;
        case GL_INVALID_VALUE:
            EQWARN << "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:
            EQWARN << "GL_INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW:
            EQWARN << "GL_STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW:
            EQWARN << "GL_STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY:
            EQWARN << "GL_OUT_OF_MEMORY"; break;
        default:
            EQWARN << "GL error 0x" << std::hex << error << std::dec; break;
    }
    
    EQWARN << ' ' << when << " in " << file << ':' << line << std::endl
           << co::base::backtrace << std::endl << co::base::enableFlush;
}                                 
}
