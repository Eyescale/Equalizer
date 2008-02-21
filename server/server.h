
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_SERVER_H
#define EQS_SERVER_H

#include "base.h"
#include "types.h"

#include <eq/client/nodeType.h>  // for TYPE_EQ_SERVER enum
#include <eq/net/command.h>      // used in inline method
#include <eq/net/commandQueue.h> // member
#include <eq/net/idHash.h>       // member
#include <eq/net/node.h>         // base class & eqsStartLocalServer declaration

/** 
 * @namespace eqs
 * @brief The Equalizer server library.
 *
 * This namespace implements the server-side functionality for the Equalizer
 * framework.
 */
namespace eqs
{
    /**
     * The Equalizer server.
     */
    class EQS_EXPORT Server : public eqNet::Node
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

        /** 
         * Map a config to this server.
         * @todo 
         * @param config the config.
         */
        void mapConfig( Config* config );
        void unmapConfig( Config* config )
            { unmapSession( (eqNet::Session*)config ); }
        
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
        eqNet::CommandQueue* getServerThreadQueue() 
            { return &_serverThreadQueue; }

    protected:
        virtual ~Server() {}

        /** @sa eqNet::Node::dispatchCommand */
        virtual bool dispatchCommand( eqNet::Command& command );

        /** @sa eqNet::Node::invokeCommand */
        virtual eqNet::CommandResult invokeCommand( eqNet::Command& command );
        
    private:
        /** The unique config identifier. */
        uint32_t _configID;

        /** The list of configurations. */
        ConfigVector   _configs;

        /** The application-allocated configurations, mapped by identifier. */
        eqNet::IDHash<Config*> _appConfigs;

        /** The receiver->main command queue. */
        eqNet::CommandQueue    _serverThreadQueue;

        /** The current state. */
        bool _running;

        /** @sa eqNet::Node::getType */
        virtual uint32_t getType() const { return eq::TYPE_EQ_SERVER; }

        void        _handleCommands(); 

        /** The command functions. */
        eqNet::CommandResult _cmdChooseConfig( eqNet::Command& command );
        eqNet::CommandResult _cmdUseConfig( eqNet::Command& command );
        eqNet::CommandResult _cmdReleaseConfig( eqNet::Command& command );
        eqNet::CommandResult _cmdShutdown( eqNet::Command& command );
    };

    std::ostream& operator << ( std::ostream& os, const Server* server );
};
#endif // EQS_SERVER_H
