
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_SERVER_H
#define EQS_SERVER_H

#include <eq/packets.h>
#include <eq/base/lock.h>
#include <eq/net/idHash.h>
#include <eq/net/node.h>
#include <eq/net/requestQueue.h>

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
         * Adds a new config to this config.
         * 
         * @param config the config.
         */
        void addConfig( Config* config ){ _configs.push_back( config ); }

        /** 
         * Removes a config from this config.
         * 
         * @param config the config
         * @return <code>true</code> if the config was removed, <code>false</code>
         *         otherwise.
         */
        bool removeConfig( Config* config );

        /** 
         * Returns the number of configs on this config.
         * 
         * @return the number of configs on this config. 
         */
        uint32_t nConfigs() const { return _configs.size(); }

        /** 
         * Gets a config.
         * 
         * @param index the config's index. 
         * @return the config.
         */
        Config* getConfig( const uint32_t index ) const 
            { return _configs[index]; }

        /** 
         * Push a request to the servers' main thread to be handled
         * asynchronously.
         * 
         * @param node the node sending the packet.
         * @param packet the command packet.
         */
        void pushRequest( eqNet::Node* node, const eqNet::Packet* packet )
            { _requestQueue.push( node, packet ); }

    protected:
        /** @sa eqNet::Node::handlePacket */
        virtual void handlePacket( eqNet::Node* node, 
                                   const eqNet::Packet* packet );
        
        /** @sa eqNet::Node::createNode */
        virtual eqBase::RefPtr<eqNet::Node> createNode();

        /** @sa eqNet::Node::handleDisconnect */
        virtual void handleDisconnect( eqNet::Node* node );

    private:
        
        /** The unique config identifier. */
        uint32_t _configID;

        /** The list of nodes. */
        std::vector<Node*>     _nodes;

        /** The list of configurations. */
        std::vector<Config*>   _configs;

        /** The application-allocated configurations, mapped by identifier. */
        eqNet::IDHash<Config*> _appConfigs;

        /** The receiver->main request queue. */
        eqNet::RequestQueue    _requestQueue;

        /** Loads the server's configuration. */
        bool _loadConfig( int argc, char **argv );

        /** Clones a configuration. */
        Config* _cloneConfig( Config* config );

        std::string _genConfigName();
        void        _handleRequests(); 

        /** The command functions. */
        void _cmdChooseConfig( eqNet::Node* node, const eqNet::Packet* packet );
        void _cmdReleaseConfig( eqNet::Node* node,const eqNet::Packet* packet );
    };

    inline std::ostream& operator << ( std::ostream& os, const Server* server )
    {
        if( !server )
        {
            os << "NULL server";
            return os;
        }

        const uint32_t nConfigs = server->nConfigs();

        os << "server " << (void*)server << " " << nConfigs << " configs";

        for( uint32_t i=0; i<nConfigs; i++ )
            os << std::endl << "    " << server->getConfig(i);

        return os;
    }
};
#endif // EQS_SERVER_H
