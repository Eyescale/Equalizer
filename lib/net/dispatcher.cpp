
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "dispatcher.h"

#include "command.h"
#include "commandQueue.h"
#include "node.h"
#include "packets.h"

#include <eq/base/log.h>

using namespace std;

namespace eq
{
namespace net
{

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
           << ", " << typeid( *this ).name() << endl;

    const uint32_t which = command->command;
#ifndef NDEBUG
    if( which >= _qTable.size( ))
    {
        EQASSERTINFO( 0, "Command " << which
                      << " higher than number of registered command handlers ("
                      << _qTable.size() << ") for object of type "
                      << typeid(*this).name() << endl );
        return false;
    }
    EQASSERT( !command.isDispatched( ));
#endif

    CommandQueue* queue = _qTable[which];
    if( queue )
        queue->push( command );
    else
    {
#ifdef NDEBUG // OPT
        _vTable[which]( command );
#else
        const CommandResult result = _vTable[which]( command );
#  ifdef EQ_SEND_TOKEN
        return( result == COMMAND_HANDLED );
#  else
        EQASSERT( result == COMMAND_HANDLED );
#  endif
#endif
    }

    return true;
}

CommandResult Dispatcher::invokeCommand( Command& command )
{
    const uint32_t which = command->command;
#ifndef NDEBUG
    if( which >= _vTable.size( ))
    {
        EQERROR << "Command " << which
                << " higher than number of registered command handlers ("
                << _vTable.size() << ") for object of type "
                << typeid(*this).name() << endl;
        return COMMAND_ERROR;
    }
    EQASSERT( command.isDispatched( ));
#endif
    return _vTable[which]( command );
}

CommandResult Dispatcher::_cmdUnknown( Command& command )
{
    EQERROR << "Unknown " << command << " for " << typeid(*this).name()
            << " @" << static_cast< void* >( this ) << endl;
    return COMMAND_ERROR;
}

}
}
