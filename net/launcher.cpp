
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "launcher.h"

#include <errno.h>
#include <unistd.h>

using namespace eqNet;
using namespace std;

bool Launcher::run( const string& command )
{
    if( command.size() == 0 )
        return false;

    Launcher launcher;
    launcher._buildCommandLine( command );
    return launcher.start();
}

void Launcher::_buildCommandLine( const string& command )
{
    const size_t length    = command.size();
    const char*  string    = command.c_str();
    bool         inTicks   = false;
    size_t       bufferPos = 0;
    char         buffer[length+1];

    _commandLine.clear();
    
    // tokenize command line
    for( size_t i=0; i<length; i++ )
    {
        const char c = string[i];
        switch( c )
        {
            case ' ':
                if( inTicks )
                    buffer[bufferPos++] = c;
                else
                {
                    buffer[bufferPos++] = '\0';
                    _commandLine.push_back( buffer );
                    bufferPos = 0;
                }
                break;

            case '\'':
                inTicks = !inTicks;
                break;

            case '\\':
                i++;
                buffer[bufferPos++] = string[i];
                break;

            default:
                buffer[bufferPos++] = c;
                break;
        }
    }

    if( bufferPos > 0 )
    {
        buffer[bufferPos++] = '\0';
        _commandLine.push_back( buffer );
    }
}

ssize_t Launcher::run()
{
    const size_t argc         = _commandLine.size();
    char*        argv[argc+1];
    
    for( size_t i=0; i<argc; i++ )
        argv[i] = (char*)_commandLine[i].c_str();

    argv[argc] = NULL;

    EQINFO << "Executing: " << argv[0] << endl;
    //return EXIT_SUCCESS;
    execvp( argv[0], argv );
    EQWARN << "Error executing '" << argv[0] << "': " << strerror(errno) <<endl;
    return EXIT_FAILURE;
}
