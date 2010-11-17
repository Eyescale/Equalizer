
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

#include "objectDeltaDataIStream.h"

#include "command.h"
#include "commands.h"
#include "objectPackets.h"

#include <eq/plugins/compressor.h>

namespace eq
{
namespace net
{
ObjectDataIStream::ObjectDataIStream()
        : _version( VERSION_INVALID )
{
    _commands.push_back( 0 ); // see getNextCommand()
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

uint32_t ObjectDataIStream::getPendingVersion() const
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
        command->release();
    _commands.pop_front();

    if( _commands.empty( ))
        return 0;
    
    return _commands.front();
}

template< typename P > bool ObjectDataIStream::_getNextBuffer(
    const uint32_t cmd, uint32_t* compressor, uint32_t* nChunks,
    const void** chunkData, uint64_t* size )
{
    const Command* command = getNextCommand();
    if( !command )
        return false;

    if( (*command)->command != cmd )
    {
        EQERROR << "Illegal command in command fifo: " << *command << std::endl;
        EQUNREACHABLE;
        return false;    
    }

    const P* packet = command->getPacket< P >();

    if( packet->dataSize == 0 ) // empty packet
        return _getNextBuffer< P >( cmd, compressor, nChunks, chunkData, size );

    *size = packet->dataSize;
    *compressor = packet->compressorName;
    *nChunks = packet->nChunks;
    *chunkData = packet->data;
    *size = packet->dataSize;
    return true;
}

template bool ObjectDataIStream::_getNextBuffer< ObjectDeltaPacket >(
    const uint32_t, uint32_t*, uint32_t*, const void**, uint64_t* );
template bool ObjectDataIStream::_getNextBuffer< ObjectInstancePacket >(
    const uint32_t, uint32_t*, uint32_t*, const void**, uint64_t* );
template bool ObjectDataIStream::_getNextBuffer< ObjectSlaveDeltaPacket >(
    const uint32_t, uint32_t*, uint32_t*, const void**, uint64_t* );

}
}
