
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "init.h"

#include "global.h"
#include "node.h"
#include "socketConnection.h"

#include <eq/base/omp.h>

#ifdef WIN32_API
#  include <direct.h>
#  define getcwd _getcwd
#endif

using namespace std;

namespace eq
{
namespace net
{

EQ_EXPORT bool init( const int argc, char** argv )
{
    if( !base::init( ))
        return false;

    EQINFO << "Log level " << base::Log::getLogLevelString() << " topics " 
           << base::Log::topics << endl;

#ifdef WIN32
    WORD    wsVersion = MAKEWORD( 2, 0 );
    WSADATA wsData;
    if( WSAStartup( wsVersion, &wsData ) != 0 )
    {
        EQERROR << "Initialization of Windows Sockets failed" 
                << base::getErrorString( GetLastError( )) << endl;
        return false;
    }
#endif

    const string programName = Global::getProgramName();
    if( programName.empty() && argc > 0 )
        Global::setProgramName( argv[0] );

    const string workDir = Global::getWorkDir();
    if( workDir.empty( ))
    {
        char cwd[MAXPATHLEN];
        getcwd( cwd, MAXPATHLEN );

        Global::setWorkDir( cwd );
    }
    return true;
}

EQ_EXPORT bool exit()
{
#ifdef WIN32
    if( WSACleanup() != 0 )
    {
        EQERROR << "Cleanup of Windows Sockets failed" 
                << base::getErrorString( GetLastError( )) << endl;
        return false;
    }
#endif
    return base::exit();
}

}
}
