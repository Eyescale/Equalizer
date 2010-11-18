
/* Copyright (c) 2005-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include "dispatcher.h"

#include "command.h"
#include "commandQueue.h"
#include "node.h"

#include <eq/base/log.h>

namespace eq
{
namespace net
{

Dispatcher::Dispatcher()
{}

Dispatcher::Dispatcher( const Dispatcher& )
{}

Dispatcher::~Dispatcher()
{}

//===========================================================================
// command handling
//===========================================================================
void Dispatcher::_registerCommand( const uint32_t command,
                                   const CommandFunc< Dispatcher >& func,
                                   CommandQueue* destinationQueue )
{
    EQASSERT( _vTable.size() == _qTable.size( ));

    if( _vTable.size() <= command )
    {
        while( _vTable.size() < command )
        {
            _vTable.push_back( 
                CommandFunc< Dispatcher >( this, &Dispatcher::_cmdUnknown ));
            _qTable.push_back( 0 );
        }

        _vTable.push_back( func );
        _qTable.push_back( destinationQueue );

        EQASSERT( _vTable.size() == command + 1 );
    }
    else
    {
        _vTable[command] = func;
        _qTable[command] = destinationQueue;
    }
}


bool Dispatcher::dispatchCommand( Command& command )
{
    EQVERB << "dispatch " << static_cast< ObjectPacket* >( command.getPacket( ))
           << ", " << typeid( *this ).name() << std::endl;

    const uint32_t which = command->command;
#ifndef NDEBUG
    if( which >= _qTable.size( ))
    {
        EQABORT( "Command " << command
                 << " higher than number of registered command handlers ("
                 << _qTable.size() << ") for object of type "
                 << base::className( this ) << std::endl );
        return false;
    }
#endif

    CommandQueue* queue = _qTable[which];
    if( queue )
    {
        queue->push( command );
        return true;
    }
    // else

#ifdef NDEBUG
    _vTable[which]( command );
#else
    const bool result = _vTable[which]( command );
    EQASSERTINFO( result, result );
#endif
    return true;
}

bool Dispatcher::invokeCommand( Command& command )
{
    const uint32_t which = command->command;
#ifndef NDEBUG
    if( which >= _vTable.size( ))
    {
        EQERROR << "Command " << which
                << " higher than number of registered command handlers ("
                << _vTable.size() << ") for object of type "
                << base::className( this ) << std::endl;
        return false;
    }
#endif
    return _vTable[which]( command );
}

bool Dispatcher::_cmdUnknown( Command& command )
{
    EQERROR << "Unknown " << command << " for " << typeid(*this).name()
            << " @" << static_cast< void* >( this ) << std::endl;
    EQUNREACHABLE;
    return false;
}

}
}
