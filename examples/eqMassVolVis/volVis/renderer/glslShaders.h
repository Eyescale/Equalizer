
/* Copyright (c) 2007,       Maxim Makhinya
   Copyright (c) 2008-2010,  Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MASS_VOL__GLSL_SHADERS_H
#define MASS_VOL__GLSL_SHADERS_H

#include <eq/client/gl.h>

namespace massVolVis
{
    class GLSLShaders
    {
    public:
        GLSLShaders();
        ~GLSLShaders();

        bool loadShaders( const std::string &vShader,
                          const std::string &fShader,
                          const GLEWContext* glewContext );

        GLhandleARB getProgram() const { return _program; }

        const GLEWContext* glewGetContext() const { return _glewContext; }

    private:
        GLhandleARB _loadShader( const std::string &shader, GLenum shaderType );
        void _printLog( GLhandleARB shader, const std::string &type );

        bool _cleanupOnError( GLhandleARB shader1 = 0, GLhandleARB shader2 = 0 );

        GLhandleARB _program;       //!< GLSL vertex and fragment shaders
        bool        _shadersLoaded; //!< flag of loaded shaders

        const GLEWContext* _glewContext;   //!< OpenGL function table
    };

}
#endif // MASS_VOL__GLSL_SHADERS_H
