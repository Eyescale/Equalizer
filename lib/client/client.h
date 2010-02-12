
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

#ifndef EQ_CLIENT_H
#define EQ_CLIENT_H

#include <eq/client/types.h>        // basic types
#include <eq/client/windowSystem.h> // WindowSystem enum

#include <eq/net/command.h>         // member
#include <eq/net/node.h>            // base class

namespace eq
{
    class CommandQueue;
    class Server;

    /** 
     * The client represents a network node of the application in the cluster.
     *
     * The methods initLocal() and exitLocal() should be used to set up and exit
     * the listening node instance for each application process.
     */
    class Client : public net::Node
    {
    public:
        /** Construct a new client. @version 1.0 */
        EQ_EXPORT Client();

        /** Destruct the client. @version 1.0 */
        EQ_EXPORT virtual ~Client();

        /** 
         * Open and connect an Equalizer server to the local client.
         *
         * The client has to be in the listening state, see initLocal().
         *
         * @param server the server.
         * @return true if the server was connected, false if not.
         * @version 1.0 
         */
        EQ_EXPORT bool connectServer( ServerPtr server );

        /** 
         * Disconnect and close the connection to an Equalizer server.
         * 
         * @param server the server.
         * @return true if the server was disconnected, false if not.
         * @version 1.0 
         */
        EQ_EXPORT bool disconnectServer( ServerPtr server );

        /**
         * @return true if the client has commands pending, false otherwise.
         * @version 1.0 
         */
        EQ_EXPORT bool hasCommands();

        /** 
         * Get and process one pending command from the node command queue.
         *
         * @version 1.0 
         */
        EQ_EXPORT void processCommand();

        /** @return the command queue to the main node thread. @internal */
        CommandQueue* getNodeThreadQueue() { return _nodeThreadQueue; }

    protected:
        /**
         * Implements the processing loop for render clients. 
         *
         * As long as the node is running, that is, between initLocal() and an
         * exit send from the server, this method executes received commands
         * using processCommand() and triggers the message pump between
         * commands.
         *
         * @sa net::Node::clientLoop(), Pipe::createMessagePump()
         * @version 1.0 
         */
        EQ_EXPORT virtual bool clientLoop();

        /** Reimplemented to also call eq::exit() on render clients. @internal*/
        EQ_EXPORT virtual bool exitClient();

        /** @internal */
        EQ_EXPORT virtual bool listen();

        /** @internal */
        EQ_EXPORT virtual bool close();

    private:
        /** The command->node command queue. */
        CommandQueue* _nodeThreadQueue;
        
        bool _running;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        /** @sa net::Node::createNode */
        EQ_EXPORT virtual net::NodePtr createNode( const uint32_t type );
        
        /** @sa net::Node::dispatchCommand */
        EQ_EXPORT virtual bool dispatchCommand( net::Command& command );

        /** @sa net::Node::invokeCommand */
        EQ_EXPORT virtual net::CommandResult invokeCommand( net::Command& );

        /** The command functions. */
        net::CommandResult _cmdExit( net::Command& command );
    };
}

#endif // EQ_CLIENT_H
