
/* Copyright (c) 2005-2012, Stefan Eilemann <eile@equalizergraphics.com> 
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

#include <lunchbox/log.h>

namespace co
{
namespace detail
{
class Dispatcher
{
public:
    /** The command handler function table. */
    std::vector< co::Dispatcher::Func > fTable;

    /** Defines a queue to which commands are dispatched from the recv. */
    std::vector< co::CommandQueue* > qTable;
};
}

Dispatcher::Dispatcher()
        : _impl( new detail::Dispatcher )
{}

Dispatcher::Dispatcher( const Dispatcher& from )
        : _impl( new detail::Dispatcher )
{}

Dispatcher::~Dispatcher()
{
    delete _impl;
}

//===========================================================================
// command handling
//===========================================================================
void Dispatcher::_registerCommand( const uint32_t command, const Func& func,
                                   CommandQueue* destinationQueue )
{
    LBASSERT( _impl->fTable.size() == _impl->qTable.size( ));

    if( _impl->fTable.size() <= command )
    {
        while( _impl->fTable.size() < command )
        {
            _impl->fTable.push_back( Func( this, &Dispatcher::_cmdUnknown ));
            _impl->qTable.push_back( 0 );
        }

        _impl->fTable.push_back( func );
        _impl->qTable.push_back( destinationQueue );

        LBASSERT( _impl->fTable.size() == command + 1 );
    }
    else
    {
        _impl->fTable[command] = func;
        _impl->qTable[command] = destinationQueue;
    }
}


bool Dispatcher::dispatchCommand( Command& command )
{
    LBASSERT( command.isValid( ));
    LBVERB << "dispatch " << command << " on " << lunchbox::className( this )
           << std::endl;

    const uint32_t which = command->command;
#ifndef NDEBUG
    if( which >= _impl->qTable.size( ))
    {
        EQABORT( "Command " << command
                 << " higher than number of registered command handlers ("
                 << _impl->qTable.size() << ") for object of type "
                 << lunchbox::className( this ) << std::endl );
        return false;
    }
#endif

    CommandQueue* queue = _impl->qTable[which];
    if( queue )
    {
        command.setDispatchFunction( _impl->fTable[which] );
        queue->push( command );
        return true;
    }
    // else

    LBCHECK( _impl->fTable[which]( command ));
    return true;
}

bool Dispatcher::_cmdUnknown( Command& command )
{
    LBERROR << "Unknown " << command << " for " << lunchbox::className( this )
            << lunchbox::backtrace << std::endl;
    EQUNREACHABLE;
    return false;
}

}
