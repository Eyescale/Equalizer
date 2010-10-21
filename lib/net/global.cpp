
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <limits>

namespace eq
{
namespace net
{

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

}


#ifndef Darwin
#  define BIG_SEND
#endif

std::string Global::_programName;
std::string Global::_workDir;
uint16_t    Global::_defaultPort = 0;
uint32_t    Global::_objectBufferSize = _getObjectBufferSize();
int32_t     Global::_iAttributes[IATTR_ALL] =
{
    100,   // INSTANCE_CACHE_SIZE
    100,   // REGISTER_SEND_QUEUE_SIZE
    100,   // REGISTER_SEND_AGE
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
};


void Global::setProgramName( const std::string& programName )
{
    _programName = programName;
}
void Global::setWorkDir( const std::string& workDir )
{
    _workDir = workDir; 
}

}
}
