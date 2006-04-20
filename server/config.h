
/* Copyright (c) 2005, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQS_CONFIG_H
#define EQS_CONFIG_H

#include <eq/client/packets.h>
#include <eq/net/session.h>

#include <iostream>
#include <vector>

namespace eqs
{
    class Compound;
    class Node;
    class Server;

    typedef eqNet::IDHash<Node*>               NodeHash;
    typedef std::vector<Node*>::const_iterator NodeIter;


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
         * Constructs a new deep copy of a config.
         */
        Config( const Config& from );

        Server* getServer() { return _server.get(); }

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
        uint32_t nNodes() const { return _nodes.size(); }

        /** 
         * Gets a node.
         * 
         * @param index the node's index. 
         * @return the node.
         */
        Node* getNode( const uint32_t index ) const
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
        uint32_t nCompounds() const { return _compounds.size(); }

        /** 
         * Gets a compound.
         * 
         * @param index the compound's index. 
         * @return the compound.
         */
        Compound* getCompound( const uint32_t index ) const
            { return _compounds[index]; }

        /** 
         * Set the maximum accepted latency for this config.
         * 
         * The latency is defined as the maximum number of frames between frame
         * begin and swap. This function may block until the new latency
         * requirement is fulfilled.
         *
         * @param latency the latency.
         */
        void setLatency( const uint32_t latency );

        /** 
         * Return the latency for this config.
         * @return the latency for this config.
         */
        uint32_t getLatency() const { return _latency; }

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
         * Sets the working directory for render client.
         * 
         * @param workDir the working directory for the  render client.
         */
        void setWorkDir( const std::string& workDir ){ _workDir = workDir; }

        /** 
         * Returns the working directory for render client.
         *
         * @return the working directory for the  render client.
         */
        const std::string& getWorkDir() const { return _workDir; }

    private:
        /** The eq server hosting the session. */
        eqBase::RefPtr<Server> _server;
        friend class Server;

        /** The list of compounds. */
        std::vector<Compound*> _compounds;

        /** The list of nodes. */
        std::vector<Node*>     _nodes;

        /** The name of the application. */
        std::string _appName;

        /** The name of the render client executable. */
        std::string _renderClient;

        /** The working directory of the render client. */
        std::string _workDir;

        /** The maximum latency between frame begin and swap, in frames. */
        uint32_t _latency;

        /** The last started frame, or 0. */
        uint32_t _frameNumber;

        /** The command functions. */
        eqNet::CommandResult _cmdRequest( eqNet::Node* node,
                                          const eqNet::Packet* packet );
        eqNet::CommandResult _reqInit( eqNet::Node* node,
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _reqExit( eqNet::Node* node,
                                       const eqNet::Packet* packet );
        eqNet::CommandResult _reqBeginFrame( eqNet::Node* node,
                                             const eqNet::Packet* packet );
        eqNet::CommandResult _reqEndFrame( eqNet::Node* node, 
                                           const eqNet::Packet* packet );

        /**
         * @name Operations
         */
        //*{
        /** common code for all constructors */
        void _construct();

        bool _init( const uint32_t initID );
        bool   _initConnectNodes();
        bool   _initNodes( const uint32_t initID );
        bool   _initPipes( const uint32_t initID );
        bool _exit();
        bool   _exitPipes();
        bool   _exitNodes();

        uint32_t _beginFrame( const uint32_t frameID, 
                              std::vector<Node*>& nodes );
        uint32_t _endFrame();

        //*}
    };

    std::ostream& operator << ( std::ostream& os, const Config* config );
};
#endif // EQS_CONFIG_H
