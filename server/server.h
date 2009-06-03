
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
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

#ifndef EQSERVER_SERVER_H
#define EQSERVER_SERVER_H

#include "base.h"
#include "types.h"
#include "visitorResult.h" // enum

#include <eq/client/nodeType.h>  // for TYPE_EQ_SERVER enum

#include <eq/net/command.h>      // used in inline method
#include <eq/net/commandQueue.h> // member
#include <eq/net/node.h>         // base class & eqsStartLocalServer declaration

namespace eq
{
/** 
 * @namespace eq::server
 * @brief The Equalizer server library.
 *
 * This namespace implements the server-side functionality for the Equalizer
 * framework. The API is not stable and certain assumptions are not documented,
 * use it at your own risk!
 */
namespace server
{
    class ConstServerVisitor;
    class ServerVisitor;

    /**
     * The Equalizer server.
     */
    class EQSERVER_EXPORT Server : public net::Node
    {
    public:
        /** 
         * Constructs a new Server.
         */
        Server();

        /** 
         * Runs the server.
         * 
         * @return <code>true</code> if the server did run successfully,
         *         <code>false</code> if not.
         */
        bool run();

        void registerConfig( Config* config );
        
        /** 
         * Add a new config to this server.
         * 
         * @param config the config.
         */
        void addConfig( Config* config );

        /** 
         * Remove a config from this config.
         * 
         * @param config the config
         * @return <code>true</code> if the config was removed,
         *         <code>false</code> otherwise.
         */
        bool removeConfig( Config* config );

        /** @return the vector of configurations. */
        const ConfigVector& getConfigs() const { return _configs; }

        /** @return the command queue to the server thread */
        net::CommandQueue* getServerThreadQueue() 
            { return &_serverThreadQueue; }

        /** 
         * Traverse this server and all children using a server visitor.
         * 
         * @param visitor the visitor.
         * @return the result of the visitor traversal.
         */
        VisitorResult accept( ServerVisitor& visitor );
        VisitorResult accept( ConstServerVisitor& visitor ) const;

    protected:
        virtual ~Server();

        /** @sa net::Node::dispatchCommand */
        virtual bool dispatchCommand( net::Command& command );

        /** @sa net::Node::invokeCommand */
        virtual net::CommandResult invokeCommand( net::Command& command );
        
    private:
        /** The unique config identifier. */
        uint32_t _configID;

        /** The list of configurations. */
        ConfigVector _configs;

        typedef stde::hash_map< uint32_t, Config* > ConfigHash;
        /** The application-allocated configurations, mapped by identifier. */
        ConfigHash _appConfigs;

        /** The receiver->main command queue. */
        net::CommandQueue    _serverThreadQueue;

        /** The current state. */
        bool _running;

        union // placeholder for binary-compatible changes
        {
            char dummy[64];
        };

        /** @sa net::Node::getType */
        virtual uint32_t getType() const { return eq::TYPE_EQ_SERVER; }

        void        _handleCommands(); 

        /** The command functions. */
        net::CommandResult _cmdChooseConfig( net::Command& command );
        net::CommandResult _cmdUseConfig( net::Command& command );
        net::CommandResult _cmdReleaseConfig( net::Command& command );
        net::CommandResult _cmdShutdown( net::Command& command );
    };

    EQSERVER_EXPORT std::ostream& operator << ( std::ostream&, const Server* );
}
}
#endif // EQSERVER_SERVER_H
