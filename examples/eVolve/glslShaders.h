
/* Copyright (c) 2007,       Maxim Makhinya
   Copyright (c) 2008,       Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#ifndef EVOLVE_GLSL_SHADERS_H
#define EVOLVE_GLSL_SHADERS_H

#include <eq/eq.h>

namespace eVolve
{
    class GLSLShaders
    {
    public:
        GLSLShaders(): _program( 0 ), _shadersLoaded( false ), _glewCtx( 0 ) {}

        bool loadShaders( const std::string &vShader,
                          const std::string &fShader,
                                GLEWContext* glewCtx );

        void unloadShaders();

        GLhandleARB  getProgram() const { return _program; }
        GLEWContext* glewGetContext()   { return _glewCtx; }

    private:
        GLhandleARB     _program;       //!< GLSL vertex and fragment shaders
        bool            _shadersLoaded; //!< flag of loaded shaders
        GLEWContext*    _glewCtx;       //!< OpenGL rendering context

        GLhandleARB _loadShader( const std::string &shader, GLenum shaderType );
        void _printLog( GLhandleARB shader, const std::string &type );
    };

}
#endif // EVOLVE_GLSL_SHADERS_H
