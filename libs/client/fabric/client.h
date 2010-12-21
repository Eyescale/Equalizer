
/* Copyright (c) 2010, Stefan Eilemann <eile@eyescale.ch> 
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

#ifndef EQFABRIC_CLIENT_H
#define EQFABRIC_CLIENT_H

#include <eq/fabric/api.h>
#include <eq/fabric/types.h>        // basic types
#include <co/localNode.h>       // base class

namespace eq
{
namespace fabric
{
    /** A client represents a network node of the application in the cluster. */
    class Client : public co::LocalNode
    {
    public:
        /** 
         * Open and connect an Equalizer server to the local client.
         *
         * The client has to be in the listening state, see initLocal().
         *
         * @param server the server.
         * @return true if the server was connected, false if not.
         * @version 1.0 
         */
        EQFABRIC_API bool connectServer( co::NodePtr server );

        /** 
         * Disconnect and close the connection to an Equalizer server.
         * 
         * @param server the server.
         * @return true if the server was disconnected, false if not.
         * @version 1.0 
         */
        EQFABRIC_API bool disconnectServer( co::NodePtr server );

        /** 
         * Get and process one pending command from the node command queue.
         *
         * @version 1.0 
         */
        EQFABRIC_API void processCommand();

        /** @return the command queue to the main node thread. @internal */
        virtual co::CommandQueue* getMainThreadQueue() = 0;

        /** @internal */
        EQFABRIC_API virtual bool dispatchCommand( co::Command& command );

    protected:
        /** Construct a new client. @internal */
        EQFABRIC_API Client();

        /** Destruct the client. @internal */
        EQFABRIC_API virtual ~Client();

    private:
        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };
    };
}
}

#endif // EQFABRIC_CLIENT_H
