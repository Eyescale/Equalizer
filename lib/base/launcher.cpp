
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "launcher.h"
#include "log.h"

#include <errno.h>
#include <sstream>
#include <unistd.h>

using namespace eqBase;
using namespace std;

// the signal handler for SIGCHILD
static void sigChildHandler( int /*signal*/ )
{
    //int status;
    //int pid = wait( &status );
    EQINFO << "Received SIGCHILD" << endl;
    signal( SIGCHLD, sigChildHandler );
}

bool Launcher::run( const string& command )
{
    if( command.size() == 0 )
        return false;

    std::vector<std::string> commandLine;
    _buildCommandLine( command, commandLine );

    signal( SIGCHLD, sigChildHandler );
    const int result = fork();
    switch( result )
    {
        case 0: // child
            break;

        case -1: // error
            EQWARN << "Could not fork child process:" << strerror( errno )
                   << endl;
            return false;

        default: // parent
            return true;
    }

    // child
    const size_t  argc         = commandLine.size();
    char*         argv[argc+1];
    ostringstream stringStream;

    for( size_t i=0; i<argc; i++ )
    {
        argv[i] = (char*)commandLine[i].c_str();
        stringStream << commandLine[i] << " ";
    }

    argv[argc] = NULL;

    EQINFO << "Executing: " << stringStream.str() << endl;
    //::exit( EXIT_SUCCESS );
    int nTries = 10;
    while( nTries-- )
    {
        execvp( argv[0], argv );
        EQWARN << "Error executing '" << argv[0] << "': " << strerror(errno)
               << endl;
        if( errno != ETXTBSY )
            break;
    }

    ::exit( EXIT_FAILURE );
    return true; // not reached
}

void Launcher::_buildCommandLine( const string& command, 
                                  std::vector<std::string>& commandLine )
{
    const size_t length    = command.size();
    const char*  string    = command.c_str();
    bool         inTicks   = false;
    size_t       bufferPos = 0;
    char         buffer[length+1];

    commandLine.clear();
    
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
                    commandLine.push_back( buffer );
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
        commandLine.push_back( buffer );
    }
}
