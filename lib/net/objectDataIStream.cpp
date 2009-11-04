
/* Copyright (c) 2007-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
#ifndef NDEBUG
    const ObjectDataPacket* packet = command.getPacket< ObjectDataPacket >();
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

}
}
