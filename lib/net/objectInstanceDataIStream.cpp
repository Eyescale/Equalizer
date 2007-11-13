
/* Copyright (c) 2007, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "objectInstanceDataIStream.h"

#include "command.h"
#include "commands.h"

#include <eq/base/idPool.h>

using namespace std;

namespace eqNet
{
ObjectInstanceDataIStream::ObjectInstanceDataIStream()
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
