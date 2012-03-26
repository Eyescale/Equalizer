
/* Copyright (c) 2008-2012, Stefan Eilemann <eile@equalizergraphics.com>
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

#include "atomic.h"
#include "rng.h"
#include "thread.h"

#include <time.h>

namespace co
{
namespace base
{
namespace
{
    static co::base::a_int32_t _initialized;
}

bool init( const int argc, char** argv )
{
#ifndef NDEBUG
    EQVERB << "Options: ";
    for( int i = 1; i < argc; ++i )
        EQVERB << argv[i] << ", ";
    EQVERB << std::endl;
#endif

    if( ++_initialized > 1 ) // not first
        return true;

    Log::instance().setThreadName( "Main" );

    const time_t now = ::time(0);
#ifdef _WIN32
    struct tm* gmt = ::gmtime( &now );
    char* gmtString = ::asctime( gmt );
#else
    struct tm gmt;
    char gmtString[32];
    ::gmtime_r( &now, &gmt );
    ::asctime_r( &gmt, gmtString );
#endif

    EQINFO << "Log level " << Log::getLogLevelString() << " topics " 
           << Log::topics << " date " << gmtString << std::endl;

    if( !RNG::_init( ))
    {
        EQERROR << "Failed to initialize random number generator" << std::endl;
        return false;
    }

    Thread::pinCurrentThread();
    return true;
}

bool exit()
{
    if( --_initialized > 0 ) // not last
        return true;
    EQASSERT( _initialized == 0 );

    Log::exit();
    return true;
}

}
}

