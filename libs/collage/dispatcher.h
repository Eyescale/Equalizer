
/* Copyright (c) 2006-2010, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef CO_DISPATCHER_H
#define CO_DISPATCHER_H

#include <co/commandFunc.h> // member

#include <co/api.h>

#include <vector>

namespace co
{
    class Connection;
    class Command;
    class CommandQueue;

    /** 
     * A helper class providing command packet dispatch functionality to
     * networked objects.
     *
     * Provides packet dispatch through a command queue and command handler
     * table.
     */
    class Dispatcher
    {
    public:
        typedef CommandFunc< Dispatcher > Func;

        CO_API Dispatcher();
        CO_API Dispatcher( const Dispatcher& from );
        CO_API virtual ~Dispatcher();

        /** NOP assignment operator. */
        const Dispatcher& operator = ( const Dispatcher& ) { return *this; }

        /** 
         * Dispatch a command from the receiver thread to the registered queue.
         * 
         * @param command the command.
         * @return true if the command was dispatched, false if not.
         * @sa registerCommand
         */
        CO_API virtual bool dispatchCommand( Command& command );

    protected:
        /** 
         * Registers a command member function for a command.
         * 
         * If the destination queue is 0, the command function is invoked
         * directly upon dispatch, otherwise it is invoked during the processing
         * of the command queue.
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
        CO_API bool _cmdUnknown( Command& command );

    private:
        CO_API void _registerCommand( const uint32_t command, 
                                      const Func& func,
                                      CommandQueue* destinationQueue );

        /** The command handler function table. */
        std::vector< Func > _fTable;
        
        /** Defines a queue to which commands are dispatched from the recv. */
        std::vector< CommandQueue* > _qTable;
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

#endif // CO_DISPATCHER_H
