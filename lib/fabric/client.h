
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

#include <eq/fabric/types.h>        // basic types
#include <eq/net/node.h>            // base class

namespace eq
{
namespace fabric
{
    /** 
     * The client represents a network node of the application in the cluster.
     *
     * The methods initLocal() and exitLocal() should be used to set up and exit
     * the listening node instance for each application process.
     */
    template< class S, class C > class Client : public net::Node
    {
    public:
        typedef base::RefPtr< S > ServerPtr; //!< The server handle

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

    protected:
        /** Construct a new client. @internal */
        EQ_EXPORT Client();

        /** Destruct the client. @internal */
        EQ_EXPORT virtual ~Client();

        /** @sa net::Node::dispatchCommand. @internal */
        EQ_EXPORT virtual bool dispatchCommand( net::Command& command );

        /** @sa net::Node::invokeCommand. @internal */
        EQ_EXPORT virtual net::CommandResult invokeCommand( net::Command& );

    private:
        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /** @sa net::Node::createNode */
        EQ_EXPORT virtual net::NodePtr createNode( const uint32_t type );
    };
}
}

#endif // EQFABRIC_CLIENT_H
