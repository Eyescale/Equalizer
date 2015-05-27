
/* Copyright (c) 2015, Stefan Eilemann <eile@equalizergraphics.com>
 *                     Daniel Nachbaur <danielnachbaur@gmail.com>
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

#ifndef EQUTIL_SHADER_H
#define EQUTIL_SHADER_H

#include <eq/api.h>
#include <eq/types.h>

namespace eq
{
namespace util
{
namespace shader
{

/**
 * Compile a shader object from a GLSL source and print errors if any.
 *
 * @param glewContext the OpenGL function table.
 * @param shader OpenGL shader object
 * @param source GLSL formatted shader source
 * @return true on successful compilation, false otherwise
 * @version 1.9
 */
EQ_API bool compile( const GLEWContext* glewContext,
                     const unsigned shader, const char* source );

/**
 * Link a shader program from a given vertex and fragment GLSL source and print
 * errors if any.
 *
 * @param glewContext the OpenGL function table.
 * @param program OpenGL shader program
 * @param vertexShaderSource GLSL formatted vertex shader source
 * @param fragmentShaderSource GLSL formatted vertex shader source
 * @return true on successful linking, false otherwise
 * @version 1.9
 */
EQ_API bool linkProgram( const GLEWContext* glewContext,
                         const unsigned program, const char* vertexShaderSource,
                         const char* fragmentShaderSource );

}
}
}

#endif // EQUTIL_SHADER_H
