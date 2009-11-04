
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "objectInstanceDataIStream.h"

#include "command.h"
#include "commands.h"

#include <eq/base/idPool.h>

using namespace std;

namespace eq
{
namespace net
{
ObjectInstanceDataIStream::ObjectInstanceDataIStream()
{
}

ObjectInstanceDataIStream::ObjectInstanceDataIStream( 
    const ObjectInstanceDataIStream& from )
        : ObjectDataIStream( from )
{}

ObjectInstanceDataIStream::~ObjectInstanceDataIStream()
{
}

bool ObjectInstanceDataIStream::getNextBuffer( const uint8_t** buffer, 
                                               uint64_t* size )
{
    const Command* command = getNextCommand();
    if( !command )
        return false;

    switch( (*command)->command )
    {
        case CMD_OBJECT_INSTANCE_DATA:
        {
            const ObjectInstanceDataPacket* packet = 
                command->getPacket< ObjectInstanceDataPacket >();
            *buffer = packet->data;
            *size   = packet->dataSize;
            return true;
        }

        case CMD_OBJECT_INSTANCE:
        {
            const ObjectInstancePacket* packet =
                command->getPacket< ObjectInstancePacket >();
            *buffer = packet->data;
            *size   = packet->dataSize;
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
