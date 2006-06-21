
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
    
    enum CommandResult
    {
        COMMAND_HANDLED,     //*< The command was handled
        COMMAND_PROPAGATE,   //*< Propagate the command to next instance
        COMMAND_RESCHEDULE,  //*< Reschedule command to be handled later
        COMMAND_ERROR        //*< An unrecoverable error occured
    };

    /** The base class for all networked objects. */
    class Base
    {
    public:
        Base( const uint32_t nCommands );
        virtual ~Base();        

        /** 
         * Handles a received command packet for this object by calling the
         * appropriate command handler function.
         * 
         * @param node the sending node.
         * @param packet the command packet.
         * @return the result of the operation.
         * @sa registerCommand
         */
        CommandResult handleCommand( Node* node, const Packet* packet );
 
    protected:

        /** The command function prototype. */
        typedef CommandResult (eqNet::Base::*CommandFcn)( Node* node, 
                                                          const Packet* packet);

        /** 
         * Registers a command member function for a command.
         * 
         * Use 'reinterpret_cast<CommandFcn>(foo)' to cast the function of the
         * derived class to the expected type.
         *
         * @param command the command.
         * @param thisPointer the <code>this</code> pointer of the caller.
         * @param handler the member function to handle the command.
         */
        void registerCommand( const uint32_t command, void* thisPointer, 
                              CommandFcn handler );

        
        /** The number of registered commands. */
        uint32_t getNCommands() const { return _nCommands; }

        /** Registers request packets waiting for a return value. */
        eqBase::RequestHandler _requestHandler;

        /** 
         * The default handler for handling commands.
         * 
         * @param node the originating node.
         * @param packet the packet.
         * @return the result of the operation.
         */
        CommandResult _cmdUnknown( Node* node, const Packet* packet );

    private:
        /** The command handler function table. */
        uint32_t    _nCommands;
        CommandFcn* _commandFunctions;
        Base**      _commandFunctionsThis;
    };
}

#endif // EQNET_BASE_H
