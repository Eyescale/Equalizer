
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "objectDataIStream.h"

#include "command.h"
#include "commands.h"
#include "objectPackets.h"

#include <co/plugins/compressor.h>

namespace co
{
ObjectDataIStream::ObjectDataIStream()
        : _version( VERSION_INVALID )
{
    reset();
}

ObjectDataIStream::~ObjectDataIStream()
{
    reset();
}

ObjectDataIStream::ObjectDataIStream( const ObjectDataIStream& from )
        : DataIStream( from )
        , _commands( from._commands )
        , _version( from._version.get( ))
{
    for( CommandDeque::const_iterator i = _commands.begin();
         i != _commands.end(); ++i )
    {
        Command* command = *i;
        if( command )
            command->retain();
    }
}

void ObjectDataIStream::reset()
{
    DataIStream::reset();

    while( !_commands.empty( ))
    {
        Command* command = _commands.front();
        if( command )
            command->release();
        _commands.pop_front();
    }

    _version = VERSION_INVALID;
    _commands.push_back( 0 ); // see getNextCommand()
}

void ObjectDataIStream::addDataPacket( Command& command )
{
    EQ_TS_THREAD( _thread );

    const ObjectDataPacket* packet = command.getPacket< ObjectDataPacket >();
#ifndef NDEBUG
    if( _commands.size() < 2 )
    {
        EQASSERT( packet->sequence == 0 );
    }
    else
    {
        const ObjectDataPacket* previous = 
            _commands.back()->getPacket< ObjectDataPacket >();
        EQASSERTINFO( packet->sequence == previous->sequence+1, 
                      packet->sequence << ", " << previous->sequence );
        EQASSERT( packet->version == previous->version );
    }
#endif

    command.retain();
    _commands.push_back( &command );
    if( packet->last )
        _setReady();
}

bool ObjectDataIStream::hasInstanceData() const
{
    for( CommandDeque::const_iterator i = _commands.begin(); 
         i != _commands.end(); ++i )
    {
        const Command* command = *i;
        if( !command )
            continue;
        
        return( (*command)->command == CMD_OBJECT_INSTANCE );
    }
    EQUNREACHABLE;
    return false;
}

size_t ObjectDataIStream::getDataSize() const
{
    size_t size = 0;
    for( CommandDeque::const_iterator i = _commands.begin(); 
         i != _commands.end(); ++i )
    {
        const Command* command = *i;
        if( !command )
            continue;

        const ObjectDataPacket* packet = 
            command->getPacket< ObjectDataPacket >();
        size += packet->dataSize;
    }
    return size;
}

uint128_t ObjectDataIStream::getPendingVersion() const
{
    if( _commands.empty( ))
        return VERSION_INVALID;

    Command* command = _commands.back();
    if( !command )
        return VERSION_INVALID;
    
    const ObjectDataPacket* packet = command->getPacket< ObjectDataPacket >();
    return packet->version;
}

const Command* ObjectDataIStream::getNextCommand()
{
    if( _commands.empty( ))
        return 0;

    // release last command
    Command* command = _commands.front();
    if( command )
    {
        command->release();
        EQASSERT( _commands.size() < 2 ||
                  (*_commands[0])->command == (*_commands[1])->command );
    }
    _commands.pop_front();

    if( _commands.empty( ))
        return 0;
    
    return _commands.front();
}

bool ObjectDataIStream::getNextBuffer( uint32_t* compressor, uint32_t* nChunks,
                                       const void** chunkData, uint64_t* size )
{
    const Command* command = getNextCommand();
    if( !command )
        return false;

    const ObjectDataPacket* packet = command->getPacket< ObjectDataPacket >();
    EQASSERT( packet->command == CMD_OBJECT_INSTANCE ||
              packet->command == CMD_OBJECT_DELTA ||
              packet->command == CMD_OBJECT_SLAVE_DELTA );

    if( packet->dataSize == 0 ) // empty packet
        return getNextBuffer( compressor, nChunks, chunkData, size );

    *size = packet->dataSize;
    *compressor = packet->compressorName;
    *nChunks = packet->nChunks;
    *size = packet->dataSize;
    switch( packet->command )
    {
      case CMD_OBJECT_INSTANCE:
        *chunkData = command->getPacket< ObjectInstancePacket >()->data;
        break;
      case CMD_OBJECT_DELTA:
        *chunkData = command->getPacket< ObjectDeltaPacket >()->data;
        break;
      case CMD_OBJECT_SLAVE_DELTA:
        *chunkData = command->getPacket< ObjectSlaveDeltaPacket >()->data;
        break;
    }
    return true;
}

}
