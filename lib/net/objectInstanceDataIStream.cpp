
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

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
    : _sequence( EQ_ID_INVALID ) // only for safety check - remove me later!
{
}

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

            EQASSERTINFO( ( _sequence==EQ_ID_INVALID && packet->sequence==0 )||
                          ( _sequence+1 == packet->sequence ),
                          "have " << _sequence << " got " << packet->sequence);
            _sequence = packet->sequence;
            return true;
        }

        case CMD_OBJECT_INSTANCE:
        {
            const ObjectInstancePacket* packet =
                command->getPacket< ObjectInstancePacket >();
            *buffer = packet->data;
            *size   = packet->dataSize;

            EQASSERTINFO( ( _sequence==EQ_ID_INVALID && packet->sequence==0 )||
                ( _sequence+1 == packet->sequence ),
                "have " << _sequence << " got " << packet->sequence);
            _sequence = packet->sequence;
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
