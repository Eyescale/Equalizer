
/* Copyright (c) 2007-2008, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

    delete _lastCommand;
    _lastCommand = 0;

    while( !_commands.empty( ))
    {
        delete _commands.front();
        _commands.pop_front();
    }

    _version = Object::VERSION_INVALID;
}

void ObjectDataIStream::addDataPacket( const Command& command )
{
    // Copy packet since it is sent to all instances
    // OPT: Do not make copy for one instance or use ref counting?

    Command* copy = new Command( command );
    _commands.push_back( copy );
}

const Command* ObjectDataIStream::getNextCommand()
{
    delete _lastCommand;
    _lastCommand = 0;
    if( _commands.empty( ))
        return 0;
    
    _lastCommand = _commands.front();
    _commands.pop_front();
    return _lastCommand; 
}
}
}
