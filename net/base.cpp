
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "base.h"

#include "node.h"
#include "packets.h"

#include <eq/base/log.h>

using namespace eqNet;
using namespace std;

Base::Base( const uint nCommands )
        : _nCommands( nCommands )
{
    _commandFunctions     = new CommandFcn[nCommands];
    _commandFunctionsThis = new Base*[nCommands];

    for( uint i=0; i<nCommands; i++ )
        registerCommand( i, this, &eqNet::Base::_cmdUnknown );
}

Base::~Base()
{
    delete [] _commandFunctions;
    delete [] _commandFunctionsThis;
}

//===========================================================================
// command handling
//===========================================================================
void Base::registerCommand( const uint command, void* thisPointer, 
                            CommandFcn handler )
{
    ASSERT( command < _nCommands );
    _commandFunctions[command]     = handler;
    _commandFunctionsThis[command] = (Base*)thisPointer;
}

void Base::handleCommand( Node* node, const Packet* packet )
{
    const uint which = packet->command;
    ASSERT( which < _nCommands );
    (_commandFunctionsThis[which]->*_commandFunctions[which])( node, packet );
}

void Base::_cmdUnknown( Node* node, const Packet* packet )
{
    ERROR << "Unknown command " << packet << " from " << node << " type "
          << typeid(*this).name() << endl;
    abort();
}
