
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
        : _lastCommand( 0 )
        , _version( Object::VERSION_INVALID )
{
}

ObjectDataIStream::~ObjectDataIStream()
{
    reset();
}

void ObjectDataIStream::reset()
{
    DataIStream::reset();

    if( _lastCommand )
        _lastCommand->release();
    _lastCommand = 0;

    while( !_commands.empty( ))
    {
        _commands.front()->release();
        _commands.pop_front();
    }

    _version = Object::VERSION_INVALID;
}

void ObjectDataIStream::addDataPacket( Command& command )
{
    command.retain();
    _commands.push_back( &command );
}

const Command* ObjectDataIStream::getNextCommand()
{
    if( _lastCommand )
        _lastCommand->release();
    _lastCommand = 0;

    if( _commands.empty( ))
        return 0;
    
    _lastCommand = _commands.front();
    _commands.pop_front();
    return _lastCommand; 
}
}
}
