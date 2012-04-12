
/* Copyright (c) 2007-2011,  Maxim Makhinya  <maxmah@gmail.com>
   Copyright (c) 2008,       Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "glslShaders.h"

namespace eVolve
{
void GLSLShaders::_printLog( GLhandleARB shader, const std::string &type )
{
    GLint length;
    glGetObjectParameterivARB( shader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length );

    if( length <= 1 )
        return;

    std::vector<GLcharARB> log;
    log.resize( length );

    GLint written;
    glGetInfoLogARB( shader, length, &written, &log[0] );
    LBERROR << "Shader error: " << type << std::endl
            << &log[0] << std::endl;

    return;
}


GLhandleARB GLSLShaders::_loadShader( const std::string &shader,
                                                 GLenum shaderType )
{
    GLhandleARB handle = glCreateShaderObjectARB( shaderType );
    const char* cstr   = shader.c_str();
    glShaderSourceARB(  handle, 1, &cstr, 0 );
    glCompileShaderARB( handle );

    GLint status;
    glGetObjectParameterivARB( handle, GL_OBJECT_COMPILE_STATUS_ARB, &status );
    if( status != GL_FALSE )
        return handle;

    _printLog( handle, "Compiling" );
    glDeleteObjectARB( handle );
    return 0;
}


bool GLSLShaders::_cleanupOnError( GLhandleARB vShader, GLhandleARB fShader )
{
    if( vShader )
        glDeleteObjectARB( vShader );

    if( fShader )
        glDeleteObjectARB( fShader );

    glDeleteObjectARB( _program );
    _program = 0;
    return false;
}


bool GLSLShaders::loadShaders( const std::string &vShader,
                               const std::string &fShader,
                               const GLEWContext* glewContext )
{
    if( _shadersLoaded )
        return true;

    LBASSERT( glewContext );
    _glewContext = glewContext;

    _program = glCreateProgramObjectARB();

    GLhandleARB vertexShader = _loadShader( vShader, GL_VERTEX_SHADER_ARB );
    if( !vertexShader )
        return _cleanupOnError();

    glAttachObjectARB( _program, vertexShader );

    GLhandleARB fragmentShader = _loadShader( fShader, GL_FRAGMENT_SHADER_ARB );
    if( !fragmentShader )
        return _cleanupOnError( vertexShader );

    glAttachObjectARB( _program, fragmentShader );

    glLinkProgramARB( _program );

    GLint status;
    glGetObjectParameterivARB( _program, GL_OBJECT_LINK_STATUS_ARB, &status );
    if( status != GL_FALSE )
    {
        _shadersLoaded = true;
        return true;
    }

    _printLog( _program, "Linking" );
    return _cleanupOnError( vertexShader, fragmentShader );
}


void GLSLShaders::unloadShaders()
{
    if( !_shadersLoaded )
        return;

    LBASSERT( _glewContext );
    LBASSERT( _program );

    glDeleteObjectARB( _program );
    _shadersLoaded = false;
    _program       = 0;
}

}
