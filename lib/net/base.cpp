
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "base.h"

#include "command.h"
#include "commandQueue.h"
#include "node.h"
#include "packets.h"

#include <eq/base/log.h>

using namespace eq::base;
using namespace std;

namespace eq
{
namespace net
{

//===========================================================================
// command handling
//===========================================================================
void Base::_registerCommand( const uint32_t command,
                             const CommandFunc<Base>& func,
                             CommandQueue* destinationQueue )
{
    EQASSERT( _vTable.size() == _qTable.size( ));

    if( _vTable.size() <= command )
    {
        while( _vTable.size() < command )
        {
            _vTable.push_back( CommandFunc<Base>( this, &Base::_cmdUnknown ));
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


bool Base::dispatchCommand( Command& command )
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
        EQASSERT( result == COMMAND_HANDLED );
#endif
    }

    return true;
}

CommandResult Base::invokeCommand( Command& command )
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
#endif
    return _vTable[which]( command );
}

CommandResult Base::_cmdUnknown( Command& command )
{
    EQERROR << "Unknown " << command << " for " << typeid(*this).name()
            << " @" << static_cast< void* >( this ) << endl;
    return COMMAND_ERROR;
}

}
}
