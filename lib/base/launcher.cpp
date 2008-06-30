
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "launcher.h"
#include "log.h"

#include <errno.h>
#include <signal.h>
#include <sstream>

using namespace eqBase;
using namespace std;

#ifndef WIN32
#  include <sys/wait.h>

// the signal handler for SIGCHILD
static void sigChildHandler( int /*signal*/ )
{
    // Get exit status to avoid zombies
    int status;
    ::wait( &status );

    // DO NOT USE cout/cerr: signal handler might be called while another cout
    //            is in progress, which will cause a deadlock due to a double
    //            flockfile() 
    // EQINFO << "Received SIGCHILD" << endl;
    
    // Re-install signal handler
    signal( SIGCHLD, sigChildHandler );
}
#endif

bool Launcher::run( const string& command )
{
    if( command.empty( ))
        return false;

#ifdef WIN32
    STARTUPINFO         startupInfo = {0};
    PROCESS_INFORMATION procInfo    = {0};
    const char*         cmdLine     = command.c_str();
    
    startupInfo.cb = sizeof(STARTUPINFO );
    const bool success = 
        CreateProcess( 0, LPSTR( cmdLine ), // program, command line
                       0, 0,                // process, thread attributes
                       FALSE,               // inherit handles
                       0,                   // creation flags
                       0,                   // environment
                       0,                   // current directory
                       &startupInfo,
                       &procInfo );

    if( !success )
    {
        EQERROR << "CreateProcess failed: " << GetLastError() << endl;
        return false;
    }

    //WaitForInputIdle( procInfo.hProcess, 1000 );
    CloseHandle( procInfo.hProcess );
    CloseHandle( procInfo.hThread );
    return true;
#else
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

    argv[argc] = 0;

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
#endif
}

#ifndef WIN32
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

            case '"':
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
#endif
