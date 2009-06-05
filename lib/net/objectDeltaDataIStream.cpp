
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "objectDeltaDataIStream.h"

#include "command.h"
#include "commands.h"

#include <eq/base/idPool.h>

using namespace std;

namespace eq
{
namespace net
{
ObjectDeltaDataIStream::ObjectDeltaDataIStream()
{
}

ObjectDeltaDataIStream::~ObjectDeltaDataIStream()
{
}

bool ObjectDeltaDataIStream::getNextBuffer( const uint8_t** buffer, 
                                            uint64_t* size )
{
    const Command* command = getNextCommand();
    if( !command )
        return false;

    switch( (*command)->command )
    {
        case CMD_OBJECT_DELTA_DATA:
        {
            const ObjectDeltaDataPacket* packet =
                command->getPacket< ObjectDeltaDataPacket >();
            *buffer = packet->delta;
            *size   = packet->deltaSize;
            return true;
        }

        case CMD_OBJECT_DELTA:
        {
            const ObjectDeltaPacket* packet =
                command->getPacket< ObjectDeltaPacket >();
            *buffer = packet->delta;
            *size   = packet->deltaSize;
            return true;
        }
        
        default: 
            EQERROR << "Illegal command in command fifo: " << *command << endl;
            EQUNREACHABLE;
    }

    return false;    
}
}
}
