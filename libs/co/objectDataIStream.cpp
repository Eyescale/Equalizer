
/* Copyright (c) 2007-2011, Stefan Eilemann <eile@equalizergraphics.com> 
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
        : _usedCommand( 0 )
{
    _reset();
}

ObjectDataIStream::~ObjectDataIStream()
{
    _reset();
}

ObjectDataIStream::ObjectDataIStream( const ObjectDataIStream& from )
        : DataIStream( from )
        , _commands( from._commands )
        , _usedCommand( 0 )
        , _version( from._version )
{
    for( CommandDequeCIter i = _commands.begin(); i != _commands.end(); ++i )
    {
        Command* command = *i;
        command->retain();
    }
}

void ObjectDataIStream::reset()
{
    DataIStream::reset();
    _reset();
}

void ObjectDataIStream::_reset()
{
    if( _usedCommand )
    {
        _usedCommand->release();
        _usedCommand = 0;
    }
    while( !_commands.empty( ))
    {
        Command* command = _commands.front();
        command->release();
        _commands.pop_front();
    }

    _version = VERSION_INVALID;
}

void ObjectDataIStream::addDataPacket( Command& command )
{
    LB_TS_THREAD( _thread );
    LBASSERT( !isReady( ));

    const ObjectDataPacket* packet = command.get< ObjectDataPacket >();
#ifndef NDEBUG
    if( _commands.empty( ))
    {
        LBASSERTINFO( packet->sequence == 0, packet );
    }
    else
    {
        const ObjectDataPacket* previous = 
            _commands.back()->get< ObjectDataPacket >();
        LBASSERTINFO( packet->sequence == previous->sequence+1, 
                      packet->sequence << ", " << previous->sequence );
        LBASSERT( packet->version == previous->version );
    }
#endif

    command.retain();
    _commands.push_back( &command );
    if( packet->last )
        _setReady();
}

bool ObjectDataIStream::hasInstanceData() const
{
    if( !_usedCommand && _commands.empty( ))
    {
        LBUNREACHABLE;
        return false;
    }

    const Command* command = _usedCommand ? _usedCommand : _commands.front();
    return( (*command)->command == CMD_OBJECT_INSTANCE );
}

NodePtr ObjectDataIStream::getMaster()
{
    if( !_usedCommand && _commands.empty( ))
        return 0;

    const Command* command = _usedCommand ? _usedCommand : _commands.front();
    return command->getNode();
}

size_t ObjectDataIStream::getDataSize() const
{
    size_t size = 0;
    for( CommandDequeCIter i = _commands.begin(); i != _commands.end(); ++i )
    {
        const Command* command = *i;
        const ObjectDataPacket* packet = 
            command->get< ObjectDataPacket >();
        size += packet->dataSize;
    }
    return size;
}

uint128_t ObjectDataIStream::getPendingVersion() const
{
    if( _commands.empty( ))
        return VERSION_INVALID;

    const Command* command = _commands.back();
    const ObjectDataPacket* packet = command->get< ObjectDataPacket >();
    return packet->version;
}

const Command* ObjectDataIStream::getNextCommand()
{
    if( _usedCommand )
        _usedCommand->release();

    if( _commands.empty( ))
        _usedCommand = 0;
    else
    {
        _usedCommand = _commands.front();
        _commands.pop_front();
    }
    return _usedCommand;
}

bool ObjectDataIStream::getNextBuffer( uint32_t* compressor, uint32_t* nChunks,
                                       const void** chunkData, uint64_t* size )
{
    const Command* command = getNextCommand();
    if( !command )
        return false;

    const ObjectDataPacket* packet = command->get< ObjectDataPacket >();
    LBASSERT( packet->command == CMD_OBJECT_INSTANCE ||
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
        *chunkData = command->get< ObjectInstancePacket >()->data;
        break;
      case CMD_OBJECT_DELTA:
        *chunkData = command->get< ObjectDeltaPacket >()->data;
        break;
      case CMD_OBJECT_SLAVE_DELTA:
        *chunkData = command->get< ObjectSlaveDeltaPacket >()->data;
        break;
    }
    return true;
}

}
