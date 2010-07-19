
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
        typedef base::RefPtr< const CL > ConstClientPtr;
        typedef std::vector< CFG* > Configs;
        typedef NF NodeFactory;

        virtual void setClient( ClientPtr client ); //!< @internal

        /** @return the local client proxy. */
        ClientPtr getClient() { return _client; }

        /** @return the local client proxy. */
        ConstClientPtr getClient() const { return _client; }

        /** @return the vector of configurations. */
        const Configs& getConfigs() const { return _configs; }

        /** @return the node factory. @internal. */
        NF* getNodeFactory() { return _nodeFactory; }

    protected:
        /** Construct a new server. */
        Server( NF* nodeFactory );

        /** Destruct this server. */
        virtual ~Server();

        /**  Add a new config to this server. @internal */
        void _addConfig( CFG* config );

        /** Remove a config from this server. @internal */
        bool _removeConfig( CFG* config );

    private:
        NF* const _nodeFactory; //!< the factory to create all child instances

        /** The local client connected to the server */
        ClientPtr _client;

        /** The list of configurations. */
        Configs _configs;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /** @sa net::Node::getType */
        virtual uint32_t getType() const { return NODETYPE_EQ_SERVER; }

        template< class, class, class, class, class, class, class >
        friend class Config;

        /* The command handler functions. */
        bool _cmdCreateConfig( net::Command& command );
        bool _cmdDestroyConfig( net::Command& command );
    };

    template< class CL, class S, class CFG, class NF > EQFABRIC_EXPORT
    std::ostream& operator << ( std::ostream&, const Server< CL, S, CFG, NF>& );
}
}

#endif // EQFABRIC_SERVER_H

