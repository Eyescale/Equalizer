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


bool checkShader( GLhandleARB shader, const string &type, const string &fn )
{
    GLint length;
    glGetObjectParameterivARB( shader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length );
    
    if( length > 0 )
    {
        vector<GLcharARB> log;
        log.resize( length );

        GLint written;
        
        glGetInfoLogARB( shader, length, &written, &log[0] );
        EQERROR << "Shader error: " << type << " " << fn << endl << &log[0] << endl;
        return false;
    }
    return true;
}

bool readShaderFile( const string &fn, vector<char> &shdrSrc )
{
    ifstream file( fn.c_str() );
    
    if( file.is_open() )
    {
        int32_t size = file.tellg();
        file.seekg( 0, ios::end );
        size = static_cast<int32_t>( file.tellg() ) - size;
        
        shdrSrc.resize( size+1 );
        
        file.seekg( 0, ios::beg );
        file.read ( &shdrSrc[0], size );
        file.close();
        
        shdrSrc[size] = '\0';
        
        return true;
    }else
    {
        EQERROR << "Can't load shader: " << fn << endl;
        return false;
    }
}

GLhandleARB loadShader( const string &filename, GLenum shaderType )
{
    vector<char> shdrSrc;
    GLhandleARB handler = glCreateShaderObjectARB( shaderType );

    if( readShaderFile( filename, shdrSrc ) )
    {
        const char *shdrSrc_ = &shdrSrc[0];
        glShaderSourceARB( handler, 1, &shdrSrc_, NULL );
    
        glCompileShaderARB( handler );

        GLint check;
        glGetObjectParameterivARB( handler, GL_OBJECT_COMPILE_STATUS_ARB, &check );
        if(!check)
            checkShader( handler, "Compiling", filename );
    }
    
    return handler;
}


bool loadShaders( const string &v_file, const string &f_file, GLhandleARB &shader )
{
    shader = glCreateProgramObjectARB();
    
    glAttachObjectARB( shader, loadShader( v_file, GL_VERTEX_SHADER_ARB   ) );
    glAttachObjectARB( shader, loadShader( f_file, GL_FRAGMENT_SHADER_ARB ) );
    
    glLinkProgramARB( shader );

    GLint check;
    glGetObjectParameterivARB( shader, GL_OBJECT_LINK_STATUS_ARB, &check );
    if( !check && !checkShader( shader, "Linker", "" ) )
        return false;
    
    glUseProgramObjectARB( shader );
    
    return true;
}

}
