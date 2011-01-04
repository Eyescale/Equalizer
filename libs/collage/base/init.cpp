
/* Copyright (c) 2008-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2010, Cedric Stalder <cedric.stalder@gmail.com>
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

#include "errorRegistry.h"
#include "global.h"
#include "log.h"
#include "pluginRegistry.h"
#include "rng.h"
#include "thread.h"

#include <fstream>

namespace co
{
namespace base
{

namespace
{
    static std::ofstream* _logFile = 0;
}

bool init( const int argc, char** argv )
{
    EQINFO << "Log level " << base::Log::getLogLevelString() << " topics " 
           << base::Log::topics << std::endl;

    for( int i=1; i<argc; ++i )
    {
        if( strcmp( "--eq-logfile", argv[i] ) == 0 )
        {
            ++i;
            if( i<argc )
            {
                EQASSERT( !_logFile );
                _logFile = new std::ofstream( argv[i] );
                if( _logFile->is_open( ))
                    Log::setOutput( *_logFile );
                else
                {
                    EQWARN << "Can't open log file " << argv[i] << ": "
                           << base::sysError << std::endl;
                    delete _logFile;
                    _logFile = 0;
                    return false;
                }
            }
        }
    }

    if( !RNG::_init( ))
    {
        EQERROR << "Failed to initialize random number generator" << std::endl;
        return false;
    }

    // init all available plugins
    PluginRegistry& pluginRegistry = Global::getPluginRegistry();
    pluginRegistry.init(); 
    Thread::pinCurrentThread();
    return true;
}

bool exit()
{
    // de-initialize registered plugins
    PluginRegistry& pluginRegistry = Global::getPluginRegistry();
    pluginRegistry.exit(); 
    co::base::Thread::removeAllListeners();
    co::base::Log::exit();

    const bool ret = RNG::_exit();
    if( _logFile )
    {
#ifdef NDEBUG
        Log::setOutput( std::cout );
#else
        Log::setOutput( std::cerr );
#endif
        _logFile->close();
        delete _logFile;
        _logFile = 0;
    }
    return ret;
}

}
}

