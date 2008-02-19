
/* Copyright (c) 2006-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_BASE_H
#define EQNET_BASE_H

#include <eq/net/commandFunc.h>

#include <eq/base/base.h>

#include <vector>

namespace eqNet
{
    class Connection;
    class Command;
    class CommandQueue;
    enum  CommandResult;

    /** 
     * The base class for all networked objects. 
     *
     * Provides packet dispatch for an object using a command handler
     * table. Handles the result of the command handlers.
     */
    class EQ_EXPORT Base
    {
    public:
        Base();
		Base( const Base& from );
        virtual ~Base();        

        /** 
         * Dispatch a command from the receiver thread to the registered queue.
         * 
         * @param command the command.
         * @return true if the command was dispatch, false if not (command will
         *         be dispatched again later)
         * @sa registerCommand
         */
        virtual bool dispatchCommand( Command& command );

        /** 
         * Handles a received command packet for this object by calling the
         * appropriate command handler function.
         * 
         * @param command the command.
         * @return the result of the operation.
         * @sa registerCommand
         */
        virtual CommandResult invokeCommand( Command& command );
 
    protected:
        /** 
         * Registers a command member function for a command.
         * 
         * @param command the command.
         * @param func the functor to handle the command.
         * @param destinationQueue the queue to which the receiver thread
         *                         dispatches the command.
         */
        template< typename T >
        void registerCommand( const uint32_t command, 
                              const CommandFunc<T>& func,
                              CommandQueue& destinationQueue );

        
        /** 
         * The default handler for handling commands.
         * 
         * @param command the command
         * @return the result of the operation.
         */
        CommandResult _cmdUnknown( Command& command );

    private:
        void _registerCommand( const uint32_t command, 
                               const CommandFunc<Base>& func,
                                CommandQueue* destinationQueue );

        /** The command handler function table. */
        std::vector< CommandFunc<Base> > _vTable;
        
        /** Defines a queue to which commands are dispatched from the recv. */
        std::vector< CommandQueue* >     _qTable;
    };

    template< typename T >
    void Base::registerCommand( const uint32_t command,
                                const CommandFunc<T>& func,
                                CommandQueue& destinationQueue )
    {
        _registerCommand( command, static_cast< CommandFunc<Base> >( func ),
                          &destinationQueue );
    }
}

#endif // EQNET_BASE_H
