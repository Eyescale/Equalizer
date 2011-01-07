
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

#include <eq/fabric/api.h>
#include <eq/fabric/types.h>     // basic typedefs
#include <eq/fabric/nodeType.h>  // for NODETYPE_EQ_SERVER enum

#include <co/types.h>

namespace eq
{
namespace fabric
{
    /** Base co::Node class for a server. @sa eq::Server */
    template< class CL, class S, class CFG, class NF, class N >
    class Server : public N
    {
    public:
        /** A reference-counted pointer to the client. */
        typedef co::base::RefPtr< CL > ClientPtr;
        /** A reference-counted const pointer to the client. */
        typedef co::base::RefPtr< const CL > ConstClientPtr;
        /** A vector of config pointers. */
        typedef std::vector< CFG* > Configs;
        /** The node factory. */
        typedef NF NodeFactory;

        virtual void setClient( ClientPtr client ); //!< @internal

        /** @return the local client proxy. @version 1.0 */
        ClientPtr getClient() { return _client; }

        /** @return the local client proxy. @version 1.0 */
        ConstClientPtr getClient() const { return _client; }

        /** @return the vector of configurations. @version 1.0 */
        const Configs& getConfigs() const { return _configs; }

        /** @internal @return the node factory. */
        NF* getNodeFactory() { return _nodeFactory; }

    protected:
        /** @internal Construct a new server. */
        Server( NF* nodeFactory );

        /** @internal Destruct this server. */
        virtual ~Server();

        /** @internal Add a new config to this server. */
        void _addConfig( CFG* config );

        /** @internal Remove a config from this server. */
        bool _removeConfig( CFG* config );

    private:
        NF* const _nodeFactory; //!< the factory to create all child instances

        /** The local client connected to the server */
        ClientPtr _client;

        /** The list of configurations. */
        Configs _configs;

        struct Private;
        Private* _private; // placeholder for binary-compatible changes

        /** @sa co::Node::getType */
        virtual uint32_t getType() const { return NODETYPE_EQ_SERVER; }

        template< class, class, class, class, class, class, class >
        friend class Config;

        /* The command handler functions. */
        bool _cmdCreateConfig( co::Command& command );
        bool _cmdDestroyConfig( co::Command& command );
    };

    template< class CL, class S, class CFG, class NF, class N > EQFABRIC_INL
    std::ostream& operator << ( std::ostream&, const Server< CL, S, CFG, NF, N>& );
}
}

#endif // EQFABRIC_SERVER_H

