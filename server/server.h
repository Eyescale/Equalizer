
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_SERVER_H
#define EQS_SERVER_H

#include <eq/client/packets.h>
#include <eq/base/lock.h>
#include <eq/net/idHash.h>
#include <eq/net/node.h>
#include <eq/net/commandQueue.h>

/** 
 * @namespace eqs
 * @brief The Equalizer server library.
 *
 * This namespace implements the server-side functionality for the Equalizer
 * framework.
 */
namespace eqs
{
    class Config;
    class Node;

    /**
     * The Equalizer server.
     */
    class Server : public eqNet::Node
    {
    public:
        /** 
         * Constructs a new Server.
         */
        Server();

        /** 
         * Runs the server.
         * 
         * @param argc the number of command line arguments.
         * @param argv the command line arguments.
         * @return <code>true</code> if the server did run successfully,
         *         <code>false</code> if not.
         */
        bool run( int argc, char **argv );

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
         * @return <code>true</code> if the config was removed, <code>false</code>
         *         otherwise.
         */
        bool removeConfig( Config* config );

        /** 
         * Return the number of configs on this config.
         * 
         * @return the number of configs on this config. 
         */
        uint32_t nConfigs() const { return _configs.size(); }

        /** 
         * Get a config.
         * 
         * @param index the config's index. 
         * @return the config.
         */
        Config* getConfig( const uint32_t index ) const 
            { return _configs[index]; }

    protected:
        virtual ~Server() {}

        /** @sa eqNet::Node::handleCommand */
        virtual eqNet::CommandResult handleCommand( eqNet::Command& command );
        
        /** @sa eqNet::Node::handleDisconnect */
        virtual void handleDisconnect( eqNet::Node* node );

        /** @sa eqNet::Node::pushCommand */
        virtual bool pushCommand( eqNet::Command& command )
            { _commandQueue.push( command ); return true; }

        /** @sa eqNet::Node::pushCommandFront */
        virtual bool pushCommandFront( eqNet::Command& command )
            { _commandQueue.pushFront( command ); return true; }

    private:
        
        /** The unique config identifier. */
        uint32_t _configID;

        /** The list of nodes. */
        std::vector<Node*>     _nodes;

        /** The list of configurations. */
        std::vector<Config*>   _configs;

        /** The application-allocated configurations, mapped by identifier. */
        eqNet::IDHash<Config*> _appConfigs;

        /** The receiver->main command queue. */
        eqNet::CommandQueue    _commandQueue;

        void        _handleCommands(); 

        /** The command functions. */
        eqNet::CommandResult _reqChooseConfig( eqNet::Command& command );
        eqNet::CommandResult _reqReleaseConfig( eqNet::Command& command );
    };

    std::ostream& operator << ( std::ostream& os, const Server* server );
};
#endif // EQS_SERVER_H
