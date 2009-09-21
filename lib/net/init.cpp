
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "init.h"

#include "global.h"
#include "node.h"
#include "socketConnection.h"

#include <eq/base/omp.h>

#ifdef WIN32_API
#  include <direct.h>
#  define getcwd _getcwd
#  ifndef MAXPATHLEN
#    define MAXPATHLEN 1024
#  endif
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
                << base::sysError << endl;
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
        Global::setWorkDir( getcwd( cwd, MAXPATHLEN ));
    }
    return true;
}

EQ_EXPORT bool exit()
{
#ifdef WIN32
    if( WSACleanup() != 0 )
    {
        EQERROR << "Cleanup of Windows Sockets failed" 
                << base::sysError << endl;
        return false;
    }
#endif
    return base::exit();
}

}
}
