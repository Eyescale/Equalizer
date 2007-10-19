/*
 *  shader.cpp
 *  volume
 *
 *  Created by huebner on 19.12.05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include <eq/eq.h>
#include "shader.h"
#include "hlp.h"

#include <iostream>
#include <fstream>

namespace eqShader
{

using namespace std;
using namespace hlpFuncs;


bool checkShader( GLhandleARB shader, const string &type )
{
    GLint length;
    glGetObjectParameterivARB( shader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length );
    
    if( length > 0 )
    {
        vector<GLcharARB> log;
        log.resize( length );

        GLint written;
        
        glGetInfoLogARB( shader, length, &written, &log[0] );
        EQERROR << "Shader error: " << type << endl << &log[0] << endl;
        return false;
    }
    return true;
}

GLhandleARB loadShader( const string &shader, GLenum shaderType )
{
    GLhandleARB handle = glCreateShaderObjectARB( shaderType );
    const char* cstr = shader.c_str();
    glShaderSourceARB( handle, 1, &cstr, NULL );
    glCompileShaderARB( handle );

    GLint check;
    glGetObjectParameterivARB( handle, GL_OBJECT_COMPILE_STATUS_ARB, &check );
    if(!check)
        checkShader( handle, "Compiling" );

    return handle;
}


bool loadShaders( const string &vShader, const string &fShader, 
                  GLhandleARB &shader )
{
    shader = glCreateProgramObjectARB();
    
    glAttachObjectARB( shader, loadShader( vShader, GL_VERTEX_SHADER_ARB   ));
    glAttachObjectARB( shader, loadShader( fShader, GL_FRAGMENT_SHADER_ARB ));
    glLinkProgramARB( shader );

    GLint check;
    glGetObjectParameterivARB( shader, GL_OBJECT_LINK_STATUS_ARB, &check );
    if( !check && !checkShader( shader, "Linking" ) )
        return false;
    
    return true;
}

}
