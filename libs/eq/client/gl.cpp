
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

#include <lunchbox/debug.h>

namespace eq
{
void debugGLError( const std::string& when, const GLenum error, 
                   const char* file, const int line )
{                                                                 
    LBWARN << lunchbox::disableFlush << "Got ";
    switch( error )
    {
        case GL_INVALID_ENUM:
            LBWARN << "GL_INVALID_ENUM"; break;
        case GL_INVALID_VALUE:
            LBWARN << "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:
            LBWARN << "GL_INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW:
            LBWARN << "GL_STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW:
            LBWARN << "GL_STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY:
            LBWARN << "GL_OUT_OF_MEMORY"; break;
        default:
            LBWARN << "GL error 0x" << std::hex << error << std::dec; break;
    }
    
    LBWARN << ' ' << when << " in " << file << ':' << line << std::endl
           << lunchbox::backtrace << std::endl << lunchbox::enableFlush;
}                                 
}
