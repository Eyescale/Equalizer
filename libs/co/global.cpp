
/* Copyright (c) 2005-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "global.h"

#include <co/base/global.h>

#include <limits>
#include <stdlib.h>

#include <vector>
#include <sstream>

namespace co
{
#define SEPARATOR '#'

#ifndef Darwin
#  define BIG_SEND
#endif

namespace
{
static uint32_t _getObjectBufferSize()
{
    const char* env = getenv( "EQ_NET_OBJECT_BUFFER_SIZE" );
    if( !env )
        return 60000;

    const int64_t size = atoi( env );
    if( size > 0 )
        return size;

    return 60000;
}

std::string _programName;
std::string _workDir;
uint16_t    _defaultPort = 0;
uint32_t    _objectBufferSize = _getObjectBufferSize();
int32_t     _iAttributes[Global::IATTR_ALL] =
{
    100,   // INSTANCE_CACHE_SIZE
    100,   // NODE_SEND_QUEUE_SIZE
    100,   // NODE_SEND_QUEUE_AGE
    10,    // RSP_TIMEOUT
    1,     // RSP_ERROR_DOWNSCALE
    5,     // RSP_ERROR_UPSCALE
    20,    // RSP_ERROR_MAXSCALE
    3,     // RSP_MIN_SENDRATE_SHIFT
#ifdef BIG_SEND
    64,    // RSP_NUM_BUFFERS
    5,     // RSP_ACK_FREQUENCY
    65000, // UDP_MTU
#else
    1024,  // RSP_NUM_BUFFERS
    17,    // RSP_ACK_FREQUENCY
    1470,  // UDP_MTU
#endif
    524288, // UDP_BUFFER_SIZE
    1,      // QUEUE_MIN_SIZE
    2,      // QUEUE_MAX_SIZE
    16384,  // RDMA_RING_BUFFER_SIZE_MB
    5000,   // RDMA_RESOLVE_TIMEOUT_MS
};
}

bool Global::fromString(const std::string& data )
{
    if (data.empty() || data[0] != SEPARATOR)
        return false;

    std::vector<uint32_t> newGlobals;
    newGlobals.reserve(IATTR_ALL);

    size_t startMarker(1u);
    size_t endMarker(1u);
    while( true )
    {
        startMarker = data.find( SEPARATOR, endMarker );
        if (startMarker == std::string::npos)
            break;

        endMarker = data.find( SEPARATOR, startMarker + 1 );
        if (endMarker == std::string::npos )
            break;

        const std::string sub = data.substr( startMarker + 1,
                                             endMarker - startMarker - 1);
        if( !sub.empty() && isdigit( sub[0] ))
            newGlobals.push_back( atoi( sub.c_str( )) );
        else
            break;
    }

    // only apply a 'complete' global list
    if( newGlobals.size() != IATTR_ALL )
        return false;

    std::copy( newGlobals.begin(), newGlobals.end(), _iAttributes );
    return true;
}

void Global::toString( std::string& data)
{
    std::stringstream stream;
    stream << SEPARATOR << SEPARATOR;
    
    for (uint32_t i = 0; i < IATTR_ALL; ++i)
        stream << _iAttributes[i] << SEPARATOR;

    stream << SEPARATOR;
    data = stream.str();
}

void Global::setProgramName( const std::string& programName )
{
    _programName = programName;
}
const std::string& Global::getProgramName()
{
    return _programName;
}

void Global::setWorkDir( const std::string& workDir )
{
    _workDir = workDir; 
}
const std::string& Global::getWorkDir()
{
    return _workDir;
}
void Global::setDefaultPort( const uint16_t port ) 
{
    _defaultPort = port;
}

uint16_t Global::getDefaultPort()
{
    return _defaultPort;
}
void Global::setObjectBufferSize( const uint32_t size )
{
    _objectBufferSize = size;
}
uint32_t Global::getObjectBufferSize()
{
    return  _objectBufferSize;
}

void Global::setIAttribute( const IAttribute attr, const int32_t value )
{
    _iAttributes[ attr ] = value;
}
int32_t Global::getIAttribute( const IAttribute attr )
{
    return _iAttributes[ attr ];
}

uint32_t Global::getTimeout()
{
    return base::Global::getIAttribute( base::Global::IATTR_ROBUSTNESS ) ? 
           base::Global::getIAttribute( base::Global::IATTR_TIMEOUT_DEFAULT ) :
           EQ_TIMEOUT_INDEFINITE;
}

uint32_t Global::getKeepaliveTimeout()
{
    const char* env = getenv( "CO_KEEPALIVE_TIMEOUT" );
    if( !env )
        return 2000; // ms

    const int64_t size = atoi( env );
    if( size == 0 )
        return 2000; // ms

    return size;
}

}
