
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

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
