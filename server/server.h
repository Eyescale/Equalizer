
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_SERVER_H
#define EQS_SERVER_H

#include <eq/packets.h>
#include <eq/net/idHash.h>
#include <eq/net/node.h>

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
        virtual bool run( int argc, char **argv );

        /** 
         * Adds a new node to this node.
         * 
         * @param node the node.
         */
        void addNode( Node* node ){ _nodes.push_back( node ); }

        /** 
         * Removes a node from this node.
         * 
         * @param node the node
         * @return <code>true</code> if the node was removed, <code>false</code>
         *         otherwise.
         */
        bool removeNode( Node* node );

        /** 
         * Returns the number of nodes on this node.
         * 
         * @return the number of nodes on this node. 
         */
        uint nNodes() const { return _nodes.size(); }

        /** 
         * Gets a node.
         * 
         * @param index the node's index. 
         * @return the node.
         */
        Node* getNode( const uint index ){ return _nodes[index]; }

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
        uint nConfigs() const { return _configs.size(); }

        /** 
         * Gets a config.
         * 
         * @param index the config's index. 
         * @return the config.
         */
        Config* getConfig( const uint index ){ return _configs[index]; }

    protected:
        /** 
         * @sa eqNet::Node::handlePacket
         */
        virtual void handlePacket( eqNet::Node* node, 
                                   const eqNet::Packet* packet );

        /** 
         * @sa eqNet::Node::handleCommand
         */
        virtual void handleCommand( eqNet::Node* node,
                                    const eqNet::NodePacket* packet );

        virtual eqNet::Node* handleConnect(
            eqBase::RefPtr<eqNet::Connection> connection );
        virtual void handleDisconnect( eqNet::Node* node );

    private:
        
        /** The unique config identifier. */
        uint _configID;

        /** Loads the server's configuration. */
        bool _loadConfig( int argc, char **argv );

        /** The command handler function table. */
        void (eqs::Server::*_cmdHandler[eq::CMD_SERVER_ALL - eqNet::CMD_NODE_CUSTOM])
            ( eqNet::Node* node, const eqNet::Packet* packet );

        void _cmdChooseConfig( eqNet::Node* node,
                               const eqNet::Packet* packet );
        void _cmdReleaseConfig( eqNet::Node* node,
                                const eqNet::Packet* packet );

        /** The list of nodes. */
        std::vector<Node*>     _nodes;

        /** The list of configurations. */
        std::vector<Config*>   _configs;

        /** The application-allocated configurations, mapped by identifier. */
        eqNet::IDHash<Config*> _appConfigs;
    };

    inline std::ostream& operator << ( std::ostream& os, Server* server )
    {
        if( !server )
        {
            os << "NULL server";
            return os;
        }

        const uint nNodes   = server->nNodes();
        const uint nConfigs = server->nConfigs();

        os << "server " << (void*)server << " " << nNodes << " nodes " 
           << nConfigs << " configs";

        for( uint i=0; i<nNodes; i++ )
            os << std::endl << "    " << server->getNode(i);
        for( uint i=0; i<nConfigs; i++ )
            os << std::endl << "    " << server->getConfig(i);

        return os;
    }
};
#endif // EQS_SERVER_H
