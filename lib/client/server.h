
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
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

#ifndef EQ_SERVER_H
#define EQ_SERVER_H

#include <eq/client/nodeType.h>  // for TYPE_EQ_SERVER enum
#include <eq/client/types.h>     // basic typedefs

#include <eq/net/node.h>         // base class

namespace eq
{
    class Client;
    class Config;
    class ConfigParams;
    class Node;
    struct ServerPacket;

    class EQ_EXPORT Server : public net::Node
    {
    public:
        /** 
         * Constructs a new server.
         */
        Server();

        /** @name Data Access */
        //*{
        void setClient( ClientPtr client );
        ClientPtr getClient(){ return _client; }

        net::CommandQueue* getNodeThreadQueue();
        net::CommandQueue* getCommandThreadQueue();
        //*}

        /** 
         * Chooses a configuration on the server.
         * 
         * @param parameters the configuration parameters
         * @return The chosen config, or <code>0</code> if no matching
         *         config was found.
         * @sa ConfigParams
         */
        Config* chooseConfig( const ConfigParams& parameters );

        /** @warning Undocumented - may not be supported in the future */
        Config* useConfig( const ConfigParams& parameters, 
                           const std::string& config );

        /** 
         * Releases the configuration.
         * 
         * The passed configuration will be destroyed by this function and is no
         * longer valid after the call.
         *
         * @param config the configuration.
         */
        void releaseConfig( Config* config );

        /** Undocumented - may not be supported in the future */
        bool shutdown();

    protected:
        /**
         * Destructs this server.
         */
        virtual ~Server();

    private:
        /** The local client connected to the server */
        ClientPtr _client;
        friend class Client; // to call invokeCommand()

        /** Process-local server */
        bool _localServer;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        /** @sa net::Node::getType */
        virtual uint32_t getType() const { return TYPE_EQ_SERVER; }

        /* The command handler functions. */
        net::CommandResult _cmdCreateConfig( net::Command& command );
        net::CommandResult _cmdDestroyConfig( net::Command& command );
        net::CommandResult _cmdChooseConfigReply( net::Command& command );
        net::CommandResult _cmdReleaseConfigReply( net::Command& command );
        net::CommandResult _cmdShutdownReply( net::Command& command );
    };

    inline std::ostream& operator << ( std::ostream& os, const Server* server )
    {
        if( !server )
        {
            os << "NULL server";
            return os;
        }

        os << "server " << (void*)server;
        return os;
    }
}

#endif // EQ_SERVER_H

