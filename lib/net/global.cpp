
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

#include "global.h"

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


std::string Global::_programName;
std::string Global::_workDir;
uint16_t    Global::_defaultPort = 0;
uint32_t    Global::_objectBufferSize = _getObjectBufferSize();


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
