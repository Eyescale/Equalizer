
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
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
     * The client represents a network node in the cluster.
     */
    class Client : public net::Node
    {
    public:
        /** 
         * Constructs a new client.
         */
        EQ_EXPORT Client();

        /**
         * Destructs the client.
         */
        EQ_EXPORT virtual ~Client();

        /** 
         * Open and connect an Equalizer server to the local client.
         * 
         * @param server the server.
         * @return true if the server was connected, false if not.
         */
        EQ_EXPORT bool connectServer( ServerPtr server );

        /** 
         * Disconnect and close the connection of an Equalizer server to the
         * local client.
         * 
         * @param server the server.
         * @return true if the server was disconnected, false if not.
         */
        EQ_EXPORT bool disconnectServer( ServerPtr server );

        /** @return true if the client has commands pending, false otherwise. */
        EQ_EXPORT bool hasCommands();

        /** 
         * Get and process one command from the node command queue. Used
         * internally to run node commands.
         */
        EQ_EXPORT void processCommand();

        /** @sa net::Node::listen() */
        EQ_EXPORT virtual bool listen();
        /** @sa net::Node::stopListening() */
        EQ_EXPORT virtual bool stopListening();

        /** 
         * Set the window system for the client's message pump, used by
         * non-threaded pipes.
         * @internal
         */
        EQ_EXPORT void setWindowSystem( const WindowSystem windowSystem );

        /** Return the command queue to the main node thread. */
        CommandQueue* getNodeThreadQueue() { return _nodeThreadQueue; }

    protected:
        /** @sa net::Node::clientLoop */
        EQ_EXPORT virtual bool clientLoop();
        /** @sa net::Node::exitClient(). */
        EQ_EXPORT virtual bool exitClient();

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
