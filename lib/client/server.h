
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

#ifndef EQ_SERVER_H
#define EQ_SERVER_H

#include <eq/client/types.h>     // basic typedefs
#include <eq/fabric/server.h>    // base class

namespace eq
{
    class Client;
    class Config;
    class ConfigParams;
    class NodeFactory;

    /**
     * Proxy object for the connection to an Equalizer server.
     *
     * The server manages the configurations for a set of Equalizer
     * applications. This proxy object is used to connect to a server and obtain
     * and release a Config from the server.
     * @sa Client::connectServer
     */
    class Server : public fabric::Server< Client, Server, Config, NodeFactory >
    {
    public:
        /** Construct a new server. */
        EQ_EXPORT Server();

        /** @name Internal */
        //@{
        virtual void setClient( ClientPtr client );
        EQ_EXPORT net::CommandQueue* getMainThreadQueue();
        EQ_EXPORT net::CommandQueue* getCommandThreadQueue();
        //@}

        /** 
         * Choose a configuration on the server.
         * 
         * @param parameters the configuration parameters
         * @return The chosen config, or 0if no matching config was found.
         * @sa ConfigParams
         */
        EQ_EXPORT Config* chooseConfig( const ConfigParams& parameters );

        /** 
         * Release a configuration.
         * 
         * The passed configuration will be destroyed by this function and is no
         * longer valid after the call.
         *
         * @param config the configuration.
         */
        EQ_EXPORT void releaseConfig( Config* config );

        /** Undocumented - may not be supported in the future */
        EQ_EXPORT bool shutdown();
        
    protected:
        /**
         * Destructs this server.
         */
        EQ_EXPORT virtual ~Server();

    private:
        /** Process-local server */
        bool _localServer;
        friend class Client;

        union // placeholder for binary-compatible changes
        {
            char dummy[32];
        };

        /* The command handler functions. */
        bool _cmdChooseConfigReply( net::Command& command );
        bool _cmdReleaseConfigReply( net::Command& command );
        bool _cmdShutdownReply( net::Command& command );
    };
}

#endif // EQ_SERVER_H

