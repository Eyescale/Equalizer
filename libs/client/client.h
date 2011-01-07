
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

#include <eq/commandQueue.h> // member
#include <eq/types.h>        // basic types
#include <eq/api.h>
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
        EQ_API Client();

        /** Destruct the client. @version 1.0 */
        EQ_API virtual ~Client();

        /** 
         * Open and connect an Equalizer server to the local client.
         *
         * The client has to be in the listening state, see initLocal().
         *
         * @param server the server.
         * @return true if the server was connected, false if not.
         * @version 1.0 
         */
        EQ_API bool connectServer( ServerPtr server );

        /** 
         * Disconnect and close the connection to an Equalizer server.
         * 
         * @param server the server.
         * @return true if the server was disconnected, false if not.
         * @version 1.0 
         */
        EQ_API bool disconnectServer( ServerPtr server );

        /** 
         * Initialize a local, listening node.
         *
         * The <code>--eq-client</code> command line option is recognized by
         * this method. It is used for remote nodes which have been
         * auto-launched by another node, e.g., remote render clients. This
         * method does not return when this command line option is present.
         *
         * @param argc the command line argument count.
         * @param argv the command line argument values.
         * @return <code>true</code> if the client was successfully initialized,
         *         <code>false</code> otherwise.
         * @version 1.0
         */
        EQ_API virtual bool initLocal( const int argc, char** argv );

        /**
         * @return true if the client has commands pending, false otherwise.
         * @version 1.0 
         */
        EQ_API bool hasCommands();

        /** @internal @return the command queue to the main node thread. */
        virtual co::CommandQueue* getMainThreadQueue()
            { return &_mainThreadQueue; }

    protected:
        /**
         * Implements the processing loop for render clients. 
         *
         * As long as the node is running, that is, between initLocal() and an
         * exit send from the server, this method executes received commands
         * using processCommand() and triggers the message pump between
         * commands.
         *
         * @sa Pipe::createMessagePump()
         * @version 1.0 
         */
        EQ_API virtual void clientLoop();

        /** Exit the process cleanly on render clients. @version 1.0 */
        EQ_API virtual void exitClient();

    private:
        /** The command->node command queue. */
        CommandQueue _mainThreadQueue;
        
        bool _running;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /** @sa co::Node::createNode */
        EQ_API virtual co::NodePtr createNode( const uint32_t type );

        bool _setupClient( const std::string& clientArgs );

        /** The command functions. */
        bool _cmdExit( co::Command& command );
    };
}

#endif // EQ_CLIENT_H
