
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_CONFIG_H
#define EQS_CONFIG_H

#include <eq/packets.h>
#include <eq/net/session.h>

#include <iostream>
#include <vector>

namespace eqs
{
    class Compound;
    class Node;

    /**
     * The config.
     */
    class Config : public eqNet::Session
    {
    public:
        /** 
         * Constructs a new Config.
         */
        Config();

        /** 
         * Adds a new node to this config.
         * 
         * @param node the node.
         */
        void addNode( Node* node );

        /** 
         * Removes a node from this config.
         * 
         * @param node the node
         * @return <code>true</code> if the node was removed, 
         *         <code>false</code> otherwise.
         */
        bool removeNode( Node* node );

        /** 
         * Returns the number of nodes on this config.
         * 
         * @return the number of nodes on this config. 
         */
        uint nNodes() const { return _nodes.size(); }

        /** 
         * Gets a node.
         * 
         * @param index the node's index. 
         * @return the node.
         */
        Node* getNode( const uint index ) const
            { return _nodes[index]; }

        /** 
         * Adds a new compound to this config.
         * 
         * @param compound the compound.
         */
        void addCompound( Compound* compound )
            { _compounds.push_back( compound ); }

        /** 
         * Removes a compound from this config.
         * 
         * @param compound the compound
         * @return <code>true</code> if the compound was removed,
         *         <code>false</code> otherwise.
         */
        bool removeCompound( Compound* compound );

        /** 
         * Returns the number of compounds on this config.
         * 
         * @return the number of compounds on this config. 
         */
        uint nCompounds() const { return _compounds.size(); }

        /** 
         * Gets a compound.
         * 
         * @param index the compound's index. 
         * @return the compound.
         */
        Compound* getCompound( const uint index ) const
            { return _compounds[index]; }

        /** 
         * Sets the identifier of this configuration.
         * 
         * @param id the identifier.
         */
        void setID( const uint id ) { _id = id; }

        /** 
         * Sets the name of the application.
         * 
         * @param name the name of the application.
         */
        void setAppName( const std::string& name )  { _appName = name; }
        
        /** 
         * Sets the name of the render client executable.
         * 
         * @param rc the name of the render client executable.
         */
        void setRenderClient( const std::string& rc ){ _renderClient = rc; }

        /** 
         * Returns the name of the render client executable.
         * 
         * @return the name of the render client executable.
         */
        const std::string& getRenderClient() const { return _renderClient; }

        /** 
         * Handles the received command packet.
         * 
         * @param node the sending node.
         * @param packet the config command packet.
         */
        void handleCommand( eqNet::Node* node,
                            const eq::ConfigPacket* packet );

    private:
        /** The list of compounds. */
        std::vector<Compound*> _compounds;

        /** The list of nodes. */
        std::vector<Node*>   _nodes;

        /** The identifier of this configuration. */
        uint _id;

        /** The name of the application. */
        std::string _appName;

        /** The name of the render client executable. */
        std::string _renderClient;

        /** The command handler function table. */
        void (eqs::Config::*_cmdHandler[eq::CMD_CONFIG_ALL])
            ( eqNet::Node* node, const eqNet::Packet* packet );

        void _cmdInit( eqNet::Node* node, const eqNet::Packet* packet );

        /**
         * @name Operations
         */
        //*{
        bool _init();
        bool _exit();
        //*/
    };

    std::ostream& operator << ( std::ostream& os, const Config* config );
};
#endif // EQS_CONFIG_H
