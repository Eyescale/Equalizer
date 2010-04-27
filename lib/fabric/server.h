
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

#ifndef EQFABRIC_SERVER_H
#define EQFABRIC_SERVER_H

#include <eq/fabric/types.h>     // basic typedefs
#include <eq/fabric/nodeType.h>  // for NODETYPE_EQ_SERVER enum

#include <eq/net/node.h>         // base class

namespace eq
{
namespace fabric
{
    template< class, class, class, class, class, class > class Config;

    /**
     * Proxy object for the connection to an Equalizer server.
     *
     * The server manages the configurations for a set of Equalizer
     * applications. This proxy object is used to connect to a server and obtain
     * and release a Config from the server.
     * @sa Client::connectServer
     */
    template< class CL, class S, class CFG, class NF >
    class Server : public net::Node
    {
    public:
        typedef base::RefPtr< CL > ClientPtr;
        typedef std::vector< CFG* > ConfigVector;

        /** @return the local client proxy. */
        ClientPtr getClient() { return _client; }

        /** @return the vector of configurations. */
        const ConfigVector& getConfigs() const { return _configs; }

        /** @return the node factory. @internal. */
        NF* getNodeFactory() { return _nodeFactory; }

    protected:
        /** Construct a new server. */
        Server( NF* nodeFactory );

        /** Destruct this server. */
        virtual ~Server();

        /** @internal */
        void setClient( ClientPtr client, net::CommandQueue* queue );

    private:
        NF* const _nodeFactory; //!< the factory to create all child instances

        /** The local client connected to the server */
        ClientPtr _client;
        //friend class Client; // to call invokeCommand()

        /** The list of configurations. */
        ConfigVector _configs;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /** @sa net::Node::getType */
        virtual uint32_t getType() const { return NODETYPE_EQ_SERVER; }

        template< class, class, class, class, class, class >
        friend class Config;

        /**  Add a new config to this server. */
        void _addConfig( CFG* config );

        /** Remove a config from this server. */
        bool _removeConfig( CFG* config );

        /* The command handler functions. */
        net::CommandResult _cmdCreateConfig( net::Command& command );
        net::CommandResult _cmdDestroyConfig( net::Command& command );
    };
}
}

#endif // EQFABRIC_SERVER_H

