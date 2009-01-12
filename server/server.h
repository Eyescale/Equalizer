
/* Copyright (c) 2005-2009, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQSERVER_SERVER_H
#define EQSERVER_SERVER_H

#include "base.h"
#include "types.h"

#include <eq/client/nodeType.h>  // for TYPE_EQ_SERVER enum
#include <eq/net/command.h>      // used in inline method
#include <eq/net/commandQueue.h> // member
#include <eq/net/idHash.h>       // member
#include <eq/net/node.h>         // base class & eqsStartLocalServer declaration

namespace eq
{
/** 
 * @namespace eq::server
 * @brief The Equalizer server library.
 *
 * This namespace implements the server-side functionality for the Equalizer
 * framework.
 */
namespace server
{
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
        ConfigVector   _configs;

        /** The application-allocated configurations, mapped by identifier. */
        net::IDHash<Config*> _appConfigs;

        /** The receiver->main command queue. */
        net::CommandQueue    _serverThreadQueue;

        /** The current state. */
        bool _running;

        /** @sa net::Node::getType */
        virtual uint32_t getType() const { return eq::TYPE_EQ_SERVER; }

        void        _handleCommands(); 

        /** The command functions. */
        net::CommandResult _cmdChooseConfig( net::Command& command );
        net::CommandResult _cmdUseConfig( net::Command& command );
        net::CommandResult _cmdReleaseConfig( net::Command& command );
        net::CommandResult _cmdShutdown( net::Command& command );
    };

    std::ostream& operator << ( std::ostream& os, const Server* server );
}
}
#endif // EQSERVER_SERVER_H
