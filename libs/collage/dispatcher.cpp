
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

#include <co/base/log.h>

namespace co
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
void Dispatcher::_registerCommand( const uint32_t command, const Func& func,
                                   CommandQueue* destinationQueue )
{
    EQASSERT( _fTable.size() == _qTable.size( ));

    if( _fTable.size() <= command )
    {
        while( _fTable.size() < command )
        {
            _fTable.push_back( Func( this, &Dispatcher::_cmdUnknown ));
            _qTable.push_back( 0 );
        }

        _fTable.push_back( func );
        _qTable.push_back( destinationQueue );

        EQASSERT( _fTable.size() == command + 1 );
    }
    else
    {
        _fTable[command] = func;
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
                 << co::base::className( this ) << std::endl );
        return false;
    }
#endif

    CommandQueue* queue = _qTable[which];
    if( queue )
    {
        command.setDispatchFunction( _fTable[which] );
        queue->push( command );
        return true;
    }
    // else

    EQCHECK( _fTable[which]( command ));
    return true;
}

bool Dispatcher::_cmdUnknown( Command& command )
{
    EQERROR << "Unknown " << command << " for " << co::base::className( this )
            << co::base::backtrace << std::endl;
    EQUNREACHABLE;
    return false;
}

}
