
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BASE_H
#define EQNET_BASE_H

#include <eq/base/base.h>
#include <eq/base/requestHandler.h>

namespace eqNet
{
    class Connection;
    class Node;
    struct Packet;
    
    /** The base class for all networked objects. */
    class Base
    {
    public:
        Base( const uint nCommands );
        virtual ~Base();        

        /** 
         * Handles a received command packet calling the appropriate command
         * handler function.
         * 
         * @param node the sending node.
         * @param packet the command packet.
         * @sa registerCommand
         */
        void handleCommand( Node* node, const Packet* packet );
 
    protected:

        /** The command function prototype. */
        typedef void (eqNet::Base::*CommandFcn)( Node* node, const Packet* packet );

        /** 
         * Registers a command member function for a command.
         * 
         * Use 'reinterpret_cast<CommandFcn>()' to cast the function of the
         * derived class to the expected type.
         *
         * @param command the command.
         * @param thisPointer the <code>this</code> pointer of the caller.
         * @param handler the member function to handle the command.
         */
        void registerCommand( const uint command, void* thisPointer, 
                              CommandFcn handler );

        /** Registers request packets waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

    private:
        /** The command handler function table. */
        uint        _nCommands;
        CommandFcn* _commandFunctions;
        Base**      _commandFunctionsThis;

        /** 
         * The default handler for handling commands.
         * 
         * @param node the originating node.
         * @param packet the packet.
         */
        void _cmdUnknown( Node* node, const Packet* packet );

    private:
    };
}

#endif // EQNET_BASE_H
