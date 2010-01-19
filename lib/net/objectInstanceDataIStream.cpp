
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *                    2010, Cedric Stalder  <cedric.stalder@gmail.com> 
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
        case CMD_OBJECT_INSTANCE:
        {
            const ObjectInstancePacket* packet =
                command->getPacket< ObjectInstancePacket >();

            *size   = packet->dataSize;

            if ( packet->compressorName != EQ_COMPRESSOR_NONE )
            {
                uint8_t* dataCompressed = const_cast<uint8_t*>( 
                                  static_cast<const uint8_t*>( packet->data ));
                _decompress( dataCompressed, buffer, packet->compressorName,
                             packet->nChunks, packet->dataSize );
                
                return true;
            }

            *buffer = packet->data;
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
