/* Copyright (c) 2007,       Maxim Makhinya
   Copyright (c) 2008,       Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "glslShaders.h"

namespace eVolve
{
bool glslShaders::_checkShader( GLhandleARB shader, const std::string &type )
{
    GLint length;
    glGetObjectParameterivARB( shader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length );

    if( length <= 0 )
        return true;
    //else Error

    std::vector<GLcharARB> log;
    log.resize( length );

    GLint written;
    glGetInfoLogARB( shader, length, &written, &log[0] );
    EQERROR << "Shader error: " << type << std::endl
            << &log[0] << std::endl;

    return false;
}


GLhandleARB glslShaders::_loadShader( const std::string &shader, GLenum shaderType )
{
    GLhandleARB handle = glCreateShaderObjectARB( shaderType );
    const char* cstr   = shader.c_str();
    glShaderSourceARB(  handle, 1, &cstr, NULL );
    glCompileShaderARB( handle );

    GLint check;
    glGetObjectParameterivARB( handle, GL_OBJECT_COMPILE_STATUS_ARB, &check );
    if( !check )
        _checkShader( handle, "Compiling" );

    return handle;
}


bool glslShaders::loadShaders( const std::string &vShader,
                               const std::string &fShader,
                                     GLEWContext* glewCtx )
{
    if( _shadersLoaded )
        return true;

    EQASSERT( glewCtx );
    _glewCtx = glewCtx;

    _program = glCreateProgramObjectARB();

    GLhandleARB vertH = _loadShader( vShader, GL_VERTEX_SHADER_ARB );
    glAttachObjectARB( _program, vertH );

    GLhandleARB horH  = _loadShader( fShader, GL_FRAGMENT_SHADER_ARB );
    glAttachObjectARB( _program, horH );

    glLinkProgramARB( _program );

    GLint check;
    glGetObjectParameterivARB( _program, GL_OBJECT_LINK_STATUS_ARB, &check );
    if( !check || !_checkShader( _program, "Linking" ))
    {
        _program = 0;
        return false;
    }

    _shadersLoaded = true;
    return true;
}

void glslShaders::unloadShaders()
{
    if( !_shadersLoaded )
        return;

    EQASSERT( _glewCtx );
    EQASSERT( _program );

    glDeleteObjectARB( _program );
    _shadersLoaded = false;
    _program       = 0;
}

}
