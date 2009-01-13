
/* Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQNET_DISPATCHER_H
#define EQNET_DISPATCHER_H

#include <eq/net/commandFunc.h> // member

#include <eq/base/base.h>

#include <vector>

namespace eq
{
namespace net
{
    class Connection;
    class Command;
    class CommandQueue;
    enum  CommandResult;

    /** 
     * A helper class providing command packet dispatch functionality to
     * networked objects.
     *
     * Provides packet dispatch through a command queue and command handler
     * table. Returns the result of the invoked command handlers.
     */
    class EQ_EXPORT Dispatcher
    {
    public:
        Dispatcher() {}
		Dispatcher( const Dispatcher& from ) {}
        virtual ~Dispatcher() {}

        /** NOP assignment operator. */
        const Dispatcher& operator = ( const Dispatcher& ) { return *this; }

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
         * If the destination queue is 0, the command function is invoked
         * directly.
         *
         * @param command the command.
         * @param func the functor to handle the command.
         * @param destinationQueue the queue to which the receiver thread
         *                         dispatches the command.
         */
        template< typename T >
        void registerCommand( const uint32_t command, 
                              const CommandFunc< T >& func,
                              CommandQueue* destinationQueue );

        
        /** 
         * The default handler for handling commands.
         * 
         * @param command the command
         * @return the result of the operation.
         */
        CommandResult _cmdUnknown( Command& command );

    private:
        void _registerCommand( const uint32_t command, 
                               const CommandFunc< Dispatcher >& func,
                               CommandQueue* destinationQueue );

        /** The command handler function table. */
        std::vector< CommandFunc< Dispatcher > > _vTable;
        
        /** Defines a queue to which commands are dispatched from the recv. */
        std::vector< CommandQueue* >       _qTable;
    };

    template< typename T >
    void Dispatcher::registerCommand( const uint32_t command,
                                const CommandFunc< T >& func,
                                CommandQueue* destinationQueue )
    {
        _registerCommand( command, CommandFunc< Dispatcher >( func ),
                          destinationQueue );
    }
}
}

#endif // EQNET_DISPATCHER_H
