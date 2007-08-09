
/* Copyright (c) 2005-2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "init.h"

#include "global.h"
#include "node.h"
#include "socketConnection.h"

#ifdef WIN32_VC
#  include <direct.h>
#  define getcwd _getcwd
#endif

using namespace eqNet;
using namespace eqBase;
using namespace std;

EQ_EXPORT bool eqNet::init( int argc, char** argv )
{
    EQINFO << "Log level " << Log::getLogLevelString() << " topics " 
           << Log::topics << endl;

    EQASSERT( argc > 0 );

#ifdef WIN32
    WORD    wsVersion = MAKEWORD( 2, 0 );
    WSADATA wsData;
    if( WSAStartup( wsVersion, &wsData ) != 0 )
    {
        EQERROR << "Initialization of Windows Sockets failed" 
                << getErrorString( GetLastError( )) << endl;
        return false;
    }
#endif

    const string programName = Global::getProgramName();
    if( programName.size() == 0  )
        Global::setProgramName( argv[0] );

    const string workDir = Global::getWorkDir();
    if( workDir.size() == 0 )
    {
        char cwd[MAXPATHLEN];
        getcwd( cwd, MAXPATHLEN );

        Global::setWorkDir( cwd );
    }
    return true;
}

EQ_EXPORT bool eqNet::exit()
{
#ifdef WIN32
    if( WSACleanup() != 0 )
    {
        EQERROR << "Cleanup of Windows Sockets failed" 
                << getErrorString( GetLastError( )) << endl;
        return false;
    }
#endif
    return true;
}
