
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

#include <eq/client/commandQueue.h> // member
#include <eq/client/types.h>        // basic types
#include <eq/fabric/client.h>       // base class

namespace eq
{
    class Server;

    /** 
     * The client represents a network node of the application in the cluster.
     *
     * The methods initLocal() and exitLocal() should be used to set up and exit
     * the listening node instance for each application process.
     *
     * @sa fabric::Client for public methods
     */
    class Client : public fabric::Client
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

        /** @return the command queue to the main node thread. @internal */
        virtual net::CommandQueue* getMainThreadQueue()
            { return &_mainThreadQueue; }

    protected:
        /** @sa net::Node::listen() @internal */
        EQ_EXPORT virtual bool listen();

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

    private:
        /** The command->node command queue. */
        CommandQueue _mainThreadQueue;
        
        bool _running;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /** @sa net::Node::createNode */
        EQ_EXPORT virtual net::NodePtr createNode( const uint32_t type );

        /** The command functions. */
        bool _cmdExit( net::Command& command );
    };
}

#endif // EQ_CLIENT_H
